/**
 * @file 5a-tickets.c
 * @author Joakim Englund (joakimenglund@protonmail.com)
 * @brief This program takes flight and booking information and 
 * create files with tickets. The base for this is 4a-tickets.c
 * @version 0.1
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief FLNode is a node in a linked list containing information about flights.
 * 
 */
typedef struct flightsInfoNode {
	int flNum;
	char dep[5];
	char des[5];
	char date[12];
	char time[7];
	int fRows;
	char *fSeatFlags;
	int bRows;
	char *bSeatFlags;
	int eRows;
	char *eSeatFlags;
	struct flightsInfoNode *next;
} FLNode;

/**
 * @brief BLNode is a node in a linked list containing information about bookings.
 * 
 */
typedef struct bookingInfoNode {
	int bNum;
	char date[12];
	char time[7];
	char dep[5];
	char des[5];
	char sClass[10];
	char fName[20];
	char surname[20];
	struct bookingInfoNode *next;
} BLNode;

FLNode* addFlights(const char *filename);
BLNode* addBookings(const char *filename);
void createTickets(FLNode *fList, BLNode *bList);
int allocateSeat(FLNode *flight, BLNode *booking, int *row, int *seat);
void createTicket(FLNode *flight, BLNode *booking, int row, int seat);
FLNode* cancelFligths(FLNode *fList);
void createSeatingMap(FLNode *fList);
FLNode* deleteFLNode(FLNode *fList, FLNode *node);
void deleteFList(FLNode *head);
void deleteBList(BLNode *head);

/**
 * main entry point of the program.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv An array of pointers to null terminated arrays of characters (command line arguments).
 *            argv[1] should be a filename of the flightdata file in csv format.
 *            argv[2] should be a filename of the booking file in csv format.
 * @param[out] returns 0 on success.
*/
int main(int argc, char **argv)
{
	// Create the linked lists.
	FLNode *fList = NULL;
	BLNode *bList = NULL;

	// Add elements to the linked lists and create tickets.
	// Commented this out and added hardcoded files to read from as that makes it easier when running via VSC.
	// TODO: Change this back bafore handing it in.
	// fList = addFlights(argv[1]);
	// bList = addBookings(argv[2]);
	 fList = addFlights("flights.csv");
	 bList = addBookings("bookings.csv");
	// Create tickets if both lists were created properly.
	if (fList && bList)
	{
		createTickets(fList, bList);
		fList = cancelFligths(fList);
		createSeatingMap(fList);
	}

	// Clean up properly.
	deleteFList(fList);
	fList = NULL;
	deleteBList(bList);
	bList = NULL;
	
	return 0;
}

/**
 * @brief This function reads data about flights from the file given as a
 * parameter, adds it to a linked list, and returns the linked list.
 * 
 * @param filename Name of file to read data from.
 * @return FLNode* Returns a pointer to a linked list on success.
 */
FLNode* addFlights(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error! Could not open the file: %s\n", filename);
		return NULL;
	}

	FLNode newNode, *fList = NULL;

	// Add nodes to the list while fscanf() reads 8 items from the file.
	while (fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^,],%d,%d,%d\n", &newNode.flNum, newNode.dep, newNode.des,
				  newNode.date, newNode.time, &newNode.fRows, &newNode.bRows, &newNode.eRows) == 8)
	{
		FLNode *newFNode = (FLNode *)malloc(sizeof(FLNode));
		memcpy(newFNode, &newNode, sizeof(FLNode));
		newFNode->fSeatFlags = (char *)malloc(newNode.fRows*sizeof(char)*7);
		memset(newFNode->fSeatFlags, 0, newNode.fRows*sizeof(char)*7);
		newFNode->bSeatFlags = (char *)malloc(newNode.bRows*sizeof(char)*7);
		memset(newFNode->bSeatFlags, 0, newNode.bRows*sizeof(char)*7);
		newFNode->eSeatFlags = (char *)malloc(newNode.eRows*sizeof(char)*7);
		memset(newFNode->eSeatFlags, 0, newNode.eRows*sizeof(char)*7);
		newFNode->next = fList;
		fList = newFNode;
	}
	fclose(fp);
	return fList;
}

/**
 * @brief This function reads data about bookings from the file given as a
 * parameter, adds it to a linked list, and returns the linked list.
 * 
 * @param filename Name of file to read data from.
 * @return BLNode* Returns a pointer to a linked list on success.
 */
BLNode* addBookings(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error! Could not open the file: %s\n", filename);
		return NULL;
	}

	BLNode newNode, *bList = NULL;

	// Add nodes to the list while fscanf() reads 8 items from the file.
	while (fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s\n", &newNode.bNum, newNode.date, newNode.time,
						 newNode.dep, newNode.des, newNode.sClass, newNode.fName, newNode.surname) == 8)
	{
		BLNode *newBNode = (BLNode *)malloc(sizeof(BLNode));
		memcpy(newBNode, &newNode, sizeof(BLNode));
		newBNode->next = bList;
		bList = newBNode;
	}
	fclose(fp);
	return bList;
}

/**
 * @brief This function contains the logic to match bookings to flights and use
 * that information to create tickets.
 * 
 * @param fList A linked list containing information about flights.
 * @param bList a linked list containing information about bookings.
 */
void createTickets(FLNode *fList, BLNode *bList)
{
	for (BLNode *BLIt = bList; BLIt != NULL; BLIt = BLIt->next)
	{
		int foundMatch = 0;
		for (FLNode *FLIt = fList; FLIt != NULL; FLIt = FLIt->next)
		{
			// If we find a matching flight for the booking..
			if (!strcmp(FLIt->dep, BLIt->dep) && !strcmp(FLIt->des, BLIt->des) && !strcmp(FLIt->date, BLIt->date) && !strcmp(FLIt->time, BLIt->time))
			{
				int row = 0, seat = 0;
				// Try to allocate a seat.
				if (allocateSeat(FLIt, BLIt, &row, &seat))
				{
					// If successful, create a ticket and flag that we got a match.
					createTicket(FLIt, BLIt, row, seat);
					foundMatch = 1;
				}
				else
				{
					// If unsuccessful, print an error message.
					fprintf(stderr, "Error! Could not find a seat in %s class for booking #%d\n", BLIt->sClass, BLIt->bNum);
				}
				break; // Break out of the flights-list loop when we find a match with a booking.
			}
		}
		if (!foundMatch)
		{
			// If we reach this part, there is no matching flight for the current booking.
			fprintf(stderr, "Error! Could not find a matching flight for booking #%d\n", BLIt->bNum);
		}
	}
}

/**
 * @brief 
 * 
 * @param flight A linked list containing information about flights. Only uses
 * the first element in the list.
 * @param booking A linked list containing information about bookings. Only uses
 * the first element in the list.
 * @param row A reference to an integer variable used to store what row is
 * booked. Used as a return value.
 * @param seat A reference to an integer variable used to store what seat is
 * booked. Used as a return value.
 * @return int Returns 0 on unsuccessful allocating of a seat, and a positive
 * integer if successful.
 */
int allocateSeat(FLNode *flight, BLNode *booking, int *row, int *seat)
{
	if (!strcmp(booking->sClass, "first"))
	{
		// Try to allocate a seat in first class
		for (int i = 0; i < flight->fRows * 7; i++)
		{
			if (flight->fSeatFlags[i] == 0)
			{
				flight->fSeatFlags[i] = 1;
				*row = (int)(i / 7) + 1;
				*seat = i + 1;
				break; // break out of the loop when we are successful.
			}
		}
	}
	else if (!strcmp(booking->sClass, "business"))
	{
		// Try to allocate a seat in business class
		for (int i = 0; i < flight->bRows * 7; i++)
		{
			if (flight->bSeatFlags[i] == 0)
			{
				flight->bSeatFlags[i] = 1;
				*row = flight->fRows + (int)(i / 7) + 1;
				*seat = flight->fRows * 7 + i + 1;
				break; // break out of the loop when we are successful.
			}
		}
	}
	else
	{
		// Try to allocate a seat in economy class
		for (int i = 0; i < flight->eRows * 7; i++)
		{
			if (flight->eSeatFlags[i] == 0)
			{
				flight->eSeatFlags[i] = 1;
				*row = flight->bRows + flight->fRows + (int)(i / 7) + 1;
				*seat = flight->bRows * 7 + flight->fRows * 7 + i + 1;
				break; // break out of the loop when we are successful.
			}
		}
	}

	return *seat; // seat will be 0 if a seat wasn't assigned, which is all we care about.
}

/**
 * @brief This function creates a ticket in form of a file.
 * 
 * @param flight A linked list containing information about flights. Only uses
 * the first element in the list.
 * @param booking A linked list containing information about bookings. Only uses
 * the first element in the list.
 * @param row An integer containing what row the ticket is booked for.
 * @param seat An integer containing what seat the ticket is booked for.
 */
void createTicket(FLNode *flight, BLNode *booking, int row, int seat)
{
	char filename[30];
	// I'm putting the tickets in a folder for now to get less clutter in my folder.
	// TODO: Remove the "putting these in own folder" part.
	sprintf(filename, "tickets/ticket-%d.txt", booking->bNum);
	FILE *fp = fopen(filename,"w");

	if(fp) {
		fprintf(fp, "BOOKING: %d\n", booking->bNum);
		fprintf(fp, "FLIGHT: %d DEPARTURE: %s DESTINATION: %s %s %s\n", flight->flNum, flight->dep, flight->des, flight->date, flight->time);
		fprintf(fp, "PASSENGER %s %s\n", booking->fName, booking->surname);
		fprintf(fp, "CLASS: %s\n", booking->sClass);
		fprintf(fp, "ROW %d SEAT %d\n", row, seat);
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Error! Could not create ticket with bookingnumber %d\n", booking->bNum);
	}
}

/*
TODO:
Möjligtvis gör om så att FLNode innehåller en variabel som indikerar om flighten
har några bokningar överhuvudtaget (sätt denna i allocateSeat() isf) för att
underlätta sökningen efter obokade flights att cancella.
*/
/**
 * @brief This function goes through a linked list that contains information
 * about flights, finds nodes that indicates that a flight is not booked, and
 * removes them from the linked list.
 * 
 * @param fList A linked list containing information about flights.
 * @return FLNode* Returns a linked list with no unbooked flights.
 */
FLNode* cancelFligths(FLNode *fList)
{
	int cancelledFlights = 0;
	FILE *fp = fopen("cancelled-flights.txt", "w");

	if (fp)
	{
		fprintf(fp, "The following flights have no passengers booked and are now cancelled:\n\n");
		fclose(fp);
	}

	for (FLNode *FLIt = fList; FLIt != NULL; FLIt = FLIt->next)
	{
		int isFlightBooked = 0;

		for (int i = 0; i < FLIt->fRows * 7; i++)
		{
			if (FLIt->fSeatFlags[i] == 1)
			{
				isFlightBooked = 1;
				break;
			}
		}

		// Just so that we don't do unneccessary work if we already found a booked seat.
		if (!isFlightBooked)
		{
			for (int i = 0; i < FLIt->bRows * 7; i++)
			{
				if (FLIt->bSeatFlags[i] == 1)
				{
					isFlightBooked = 1;
					break;
				}
			}

			// Just so that we don't do unneccessary work if we already found a booked seat.
			if (!isFlightBooked)
			{
				for (int i = 0; i < FLIt->eRows * 7; i++)
				{
					if (FLIt->eSeatFlags[i] == 1)
					{
						isFlightBooked = 1;
						break;
					}
				}
			}
		}

		if (!isFlightBooked)
		{
			cancelledFlights++;
			FILE *fp = fopen("cancelled-flights.txt", "a");
			
			if(fp) {
				fprintf(fp, "%d) Flight %d, Departure %s, Destination %s, Date %s, Time %s\n", cancelledFlights, FLIt->flNum, FLIt->dep, FLIt->des, FLIt->date, FLIt->time);
				fclose(fp);
				fList = deleteFLNode(fList, FLIt);
			}
			else
			{
				fprintf(stderr, "Error! Could not open the file containing cancelled flights!\n");
			}
		}
	}
	return fList;
}

/**
 * @brief This function removes a node from a linked list of type FLNode.
 * 
 * @param fList A linked list containing information about flights.
 * @param node A node in a linked list that is going to be removed.
 * @return FLNode* Returns a linked list with the node removed.
 */
FLNode* deleteFLNode(FLNode *fList, FLNode *node)
{
	if (fList == node)
	{
		fList = fList->next;
	}
	else
	{
		FLNode *FLIt = fList;
		while (FLIt->next != node)
		{
			FLIt = FLIt->next;
		}
		FLIt->next = FLIt->next->next;
	}
	free(node->fSeatFlags);
	free(node->bSeatFlags);
	free(node->eSeatFlags);
	free(node);

	return fList;
}

/*
TODO:
Seating-report ska innehålla seating-map för alla flights, alternativt kan
programmet ta ett argument för om man vill ha en rapport för alla, eller en
rapport per flight.
*/
/**
 * @brief This function creates a seating-report.txt file containing a seating-map
 * for every flight that has passengers.
 * 
 * @param fList A linked list containing information about flights.
 */
void createSeatingMap(FLNode *fList)
{
	FILE *fp = fopen("seating-report.txt", "w");

	if (fp)
	{
		for (FLNode *FLIt = fList; FLIt != NULL; FLIt = FLIt->next)
		{
			fprintf(fp, "Flight %d, Departure %s, Destination %s, Date %s, Time %s\n", FLIt->flNum, FLIt->dep, FLIt->des, FLIt->date, FLIt->time);
			fprintf(fp, "first class\n");
			for (int i = 0; i < FLIt->fRows; i++)
			{
				fprintf(fp, "[%d][%d] [%d][%d][%d] [%d][%d]", FLIt->fSeatFlags[i * 7], FLIt->fSeatFlags[i * 7 + 1], FLIt->fSeatFlags[i * 7 + 2],
							 FLIt->fSeatFlags[i * 7 + 3], FLIt->fSeatFlags[i * 7 + 4], FLIt->fSeatFlags[i * 7 + 5], FLIt->fSeatFlags[i * 7 + 6]);
				fprintf(fp, "\n");
			}
			fprintf(fp, "business class\n");
			for (int i = 0; i < FLIt->bRows; i++)
			{
				fprintf(fp, "[%d][%d] [%d][%d][%d] [%d][%d]", FLIt->bSeatFlags[i * 7], FLIt->bSeatFlags[i * 7 + 1], FLIt->bSeatFlags[i * 7 + 2],
							 FLIt->bSeatFlags[i * 7 + 3], FLIt->bSeatFlags[i * 7 + 4], FLIt->bSeatFlags[i * 7 + 5], FLIt->bSeatFlags[i * 7 + 6]);
				fprintf(fp, "\n");
			}
			fprintf(fp, "economy class\n");
			for (int i = 0; i < FLIt->eRows; i++)
			{
				fprintf(fp, "[%d][%d] [%d][%d][%d] [%d][%d]", FLIt->eSeatFlags[i * 7], FLIt->eSeatFlags[i * 7 + 1], FLIt->eSeatFlags[i * 7 + 2],
							 FLIt->eSeatFlags[i * 7 + 3], FLIt->eSeatFlags[i * 7 + 4], FLIt->eSeatFlags[i * 7 + 5], FLIt->eSeatFlags[i * 7 + 6]);
				fprintf(fp, "\n");
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
}

/**
 * @brief This function calls free() on every block of memory that was allocated
 * with malloc in a linked list of the FLNode type.
 * 
 * @param head Pointer to the linked list we want to delete properly.
 */
void deleteFList(FLNode *head)
{
	FLNode *temp = NULL;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		free(temp->fSeatFlags);
		free(temp->bSeatFlags);
		free(temp->eSeatFlags);
		free(temp);
	}
}

/**
 * @brief This function calls free() on every block of memory that was allocated
 * with malloc in a linked list of the BLNode type.
 * 
 * @param head Pointer to the linked list we want to delete properly.
 */
void deleteBList(BLNode *head)
{
	BLNode *temp = NULL;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		free(temp);
	}
}
