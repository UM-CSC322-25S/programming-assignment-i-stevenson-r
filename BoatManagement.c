#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 128
#define MAX_BOAT_LENGTH 100   // Maximum allowed boat length in feet
#define MAX_BOATS 120         // Maximum number of boats

// Defining the NameString as a char array
typedef char NameString[MAX_NAME_LENGTH];

// Enum for the different types of boat places
typedef enum {
    slip,
    land,
    trailor, 
    storage
} PlaceType;

// Union for storing place-specific information
typedef union {
    int SlipNumber;
    char BayLetter;
    char TrailorLicenseTag[7];
    int StorageSpaceNumber;
} PlaceExtra;

// Boat structure definition
typedef struct {
    NameString BoatName;
    int BoatLength; // in feet
    PlaceType TypeOfPlace;
    PlaceExtra PlaceInformation;
    double AmountOwed;
} Boat;

//--------------------------------------------------------------------------------------------------

// Sort function for Bubble Sort algorithm
void sortNames(Boat *boats[], int boatCount) {
    int Swapped = 1;
    int Index;
    Boat* Temporary;
    while (Swapped) {
        Swapped = 0;
        for (Index = 0; Index < boatCount - 1; Index++) {
            if (strcmp(boats[Index]->BoatName, boats[Index + 1]->BoatName) > 0) {
                Temporary = boats[Index];
                boats[Index] = boats[Index + 1];
                boats[Index + 1] = Temporary;
                Swapped = 1;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------

// Function to dynamically allocate memory
void* Malloc(size_t Size) {
    void* Memory;
    if ((Memory = malloc(Size)) == NULL) {
        perror("Cannot malloc");
        exit(EXIT_FAILURE);
    }
    return Memory;
}

//--------------------------------------------------------------------------------------------------

int loadBoatFromCSVLine(Boat *newBoat, char *line) {
    // Buffers for place type and place-specific information
    char placeType[MAX_NAME_LENGTH];
//    char remainingData[MAX_NAME_LENGTH];
    // First, parse the boat name and boat length
    if (sscanf(line, "%127[^,],%d", newBoat->BoatName, &newBoat->BoatLength) != 2) {
        printf("Invalid line (boat name and length): %s\n", line);
        return 0;  // Invalid line if boat name and length aren't parsed correctly
    }
    // Now, move past the boat name and length (so we can parse the rest)
    char *remainingDataPtr = strchr(line, ',');  // Skip over the boat name
    if (remainingDataPtr) remainingDataPtr++;  // Move past the comma
    remainingDataPtr = strchr(remainingDataPtr, ',');  // Skip over the boat length
    if (remainingDataPtr) remainingDataPtr++;  // Move past the comma
    // Now, parse the place type and the associated place-specific information
    if (sscanf(remainingDataPtr, "%127[^,]", placeType) != 1) {
        printf("Invalid line (place type): %s\n", remainingDataPtr);
        return 0;  // Invalid line if place type isn't parsed correctly
    }
    // Handle the place type and assign place-specific data
    if (strcmp(placeType, "slip") == 0) {
        newBoat->TypeOfPlace = slip;
        // Now parse the slip number after the place type
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        if (sscanf(remainingDataPtr, "%d", &newBoat->PlaceInformation.SlipNumber) != 1) {
            printf("Invalid slip number\n");
            return 0;  // Invalid slip number if not parsed correctly
        }
      // printf("Slip Number: %d\n", newBoat->PlaceInformation.SlipNumber);
    } else if (strcmp(placeType, "land") == 0) {
        newBoat->TypeOfPlace = land;
        // Now parse the bay letter for land
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        if (sscanf(remainingDataPtr, "%c", &newBoat->PlaceInformation.BayLetter) != 1) {
            printf("Invalid bay letter\n");
            return 0;  // Invalid bay letter if not parsed correctly
        }
    //    printf("Bay Letter: %c\n", newBoat->PlaceInformation.BayLetter);
    } else if (strcmp(placeType, "trailor") == 0) {
        newBoat->TypeOfPlace = trailor;
        // Now parse the trailer license tag
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        if (sscanf(remainingDataPtr, "%6s", newBoat->PlaceInformation.TrailorLicenseTag) != 1) {
            printf("Invalid trailer license tag\n");
            return 0;  // Invalid trailer license tag if not parsed correctly
        }
  //      printf("Tag: %s\n", newBoat->PlaceInformation.TrailorLicenseTag);
    } else if (strcmp(placeType, "storage") == 0) {
        newBoat->TypeOfPlace = storage;
        // Now parse the storage space number
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        if (sscanf(remainingDataPtr, "%d", &newBoat->PlaceInformation.StorageSpaceNumber) != 1) {
            printf("Invalid storage space number\n");
            return 0;  // Invalid storage space number if not parsed correctly
        }
//        printf("Space Number: %d\n", newBoat->PlaceInformation.StorageSpaceNumber);
    } else {
        printf("Unknown place type: %s\n", placeType);
        return 0;  // Unknown place type
    }
    // After determining the place type and parsing the specific place information, parse the AmountOwed
    remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place information)
    if (sscanf(remainingDataPtr, "%lf", &newBoat->AmountOwed) != 1) {
        printf("Invalid amount owed\n");
        return 0;  // Invalid amount owed if not parsed correctly
    }
    return 1;  // Successfully parsed the boat data
}
int loadBoatsFromCSV(const char *filename, Boat **boats) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;  // Return error code if file can't be opened
    }
    char line[MAX_NAME_LENGTH + 50];
    int boatCount = 0;
    while (fgets(line, sizeof(line), file)) {
        Boat *newBoat = malloc(sizeof(Boat));  // Dynamically allocate memory for each boat
        
        if (!loadBoatFromCSVLine(newBoat, line)) {
            printf("Skipping invalid boat data: %s\n", line);
            free(newBoat);  // Free the invalid boat data
            continue;
        }
        boats[boatCount] = newBoat;  // Store the boat pointer in the array
        boatCount++;
        // Limit the number of boats loaded (if necessary)
        if (boatCount >= MAX_BOATS) {
            printf("Maximum boat count reached.\n");
            break;
        }
    }
    fclose(file);
    return boatCount;
}

//--------------------------------------------------------------------------------------------------

// Function to save boat data to the CSV file
void saveBoatsToCSV(const char *filename, Boat **boats, int boatCount) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < boatCount; i++) {
        // Write each boat's data in CSV format
        fprintf(file, "%s,%d", 
                boats[i]->BoatName,
                boats[i]->BoatLength);
        if (boats[i]->TypeOfPlace == slip) {
            fprintf(file, ",slip,%d,%.2f\n",  boats[i]->PlaceInformation.SlipNumber,
boats[i]->AmountOwed);
        } else if(boats[i]->TypeOfPlace == land) {
            fprintf(file, ",land,%c,%.2f\n",  boats[i]->PlaceInformation.BayLetter, boats[i]->AmountOwed);
        } else if(boats[i]->TypeOfPlace == trailor) {
            fprintf(file, ",trailor,%s,%.2f\n",  boats[i]->PlaceInformation.TrailorLicenseTag, boats[i]->AmountOwed);
        } else {
            fprintf(file, ",storage,%d,%.2f\n",  boats[i]->PlaceInformation.StorageSpaceNumber, boats[i]->AmountOwed);
        } 
    }
    sortNames(boats, boatCount);
    fclose(file);
}

//--------------------------------------------------------------------------------------------------

void displayBoatInventory(Boat *boatList[], int boatCount) {
    sortNames(boatList, boatCount);
    for (int i = 0; i < boatCount; i++) {
        Boat *boat = boatList[i];
        
        // Print basic boat details
        printf("%-20s %-3d\' %-8s ", boat->BoatName, boat->BoatLength,
               (boat->TypeOfPlace == slip) ? "slip" :
               (boat->TypeOfPlace == land) ? "land" :
               (boat->TypeOfPlace == trailor) ? "trailor" :
               "storage");
        // Print place-specific information based on place type
        if (boat->TypeOfPlace == slip) {
            printf("#%-7d ", boat->PlaceInformation.SlipNumber);  // Print slip number
        } else if (boat->TypeOfPlace == land) {
            printf("%-7c ", boat->PlaceInformation.BayLetter);  // Print bay letter for land
        } else if (boat->TypeOfPlace == trailor) {
            printf("%-7s ", boat->PlaceInformation.TrailorLicenseTag);  // Print trailer license tag
        } else if (boat->TypeOfPlace == storage) {
            printf("#%-7d ", boat->PlaceInformation.StorageSpaceNumber);  // Print storage space number
        }
        // Print the amount owed
        printf("Owes $%6.2f\n", boat->AmountOwed);
    }
}

//--------------------------------------------------------------------------------------------------

// Function to parse and add boat details in CSV format
void addBoat(Boat **boats, int *boatCount) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }

    Boat newBoat;
    char csvLine[MAX_NAME_LENGTH * 4]; // Buffer to store the CSV string
    char placeType[MAX_NAME_LENGTH];
    char placeSpecificDetails[MAX_NAME_LENGTH];

    // Clear the buffer before waiting for user input
    while (getchar() != '\n');  // This consumes any leftover newline character

    // Prompt user to enter boat details in CSV format
    printf("Enter boat details in CSV format (e.g., BoatName,BoatLength,PlaceType,PlaceSpecificDetails,AmountOwed):\n");
    fgets(csvLine, sizeof(csvLine), stdin);
    csvLine[strcspn(csvLine, "\n")] = '\0';  // Remove trailing newline

    // Tokenize the CSV input
    char *token = strtok(csvLine, ",");
    
    // Boat Name
    if (token != NULL) {
        strncpy(newBoat.BoatName, token, MAX_NAME_LENGTH - 1);
        newBoat.BoatName[MAX_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    }

    // Boat Length
    token = strtok(NULL, ",");
    if (token != NULL) {
        newBoat.BoatLength = atoi(token);
    }

    // Place Type
    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(placeType, token, MAX_NAME_LENGTH - 1);
        placeType[MAX_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    }

    // Place Specific Details
    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(placeSpecificDetails, token, MAX_NAME_LENGTH - 1);
        placeSpecificDetails[MAX_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    }

    // Amount Owed
    token = strtok(NULL, ",");
    if (token != NULL) {
        newBoat.AmountOwed = atof(token);
    }

    // Determine the place type and assign specific details
    if (strcmp(placeType, "slip") == 0) {
        newBoat.TypeOfPlace = slip;
        newBoat.PlaceInformation.SlipNumber = atoi(placeSpecificDetails);
    } else if (strcmp(placeType, "land") == 0) {
        newBoat.TypeOfPlace = land;
        newBoat.PlaceInformation.BayLetter = placeSpecificDetails[0]; // Assuming single character
    } else if (strcmp(placeType, "trailor") == 0) {
        newBoat.TypeOfPlace = trailor;
        strncpy(newBoat.PlaceInformation.TrailorLicenseTag, placeSpecificDetails, 6);
        newBoat.PlaceInformation.TrailorLicenseTag[6] = '\0'; // Ensure null termination
    } else if (strcmp(placeType, "storage") == 0) {
        newBoat.TypeOfPlace = storage;
        newBoat.PlaceInformation.StorageSpaceNumber = atoi(placeSpecificDetails);
    } else {
        printf("Invalid place type.\n");
        return;
    }

    // Store the new boat
    boats[*boatCount] = (Boat *)malloc(sizeof(Boat));
    *boats[*boatCount] = newBoat;  // Copy the new boat into the array
    (*boatCount)++;

    printf("Boat added successfully.\n");
}

//--------------------------------------------------------------------------------------------------

// Function to remove a boat by name
void removeBoat(Boat **boats, int *boatCount) {
    char boatName[MAX_NAME_LENGTH];
    printf("Enter boat name to remove: ");
    scanf(" %127[^\n]", boatName);
    int indexToRemove = -1;
    for (int i = 0; i < *boatCount; i++) {
        if (strcasecmp(boats[i]->BoatName, boatName) == 0) {
            indexToRemove = i;
            break;
        }
    }
    if (indexToRemove == -1) {
        printf("Boat with name '%s' not found.\n", boatName);
        return;
    }
    free(boats[indexToRemove]);
    // Shift all the remaining boats in the array
    for (int i = indexToRemove; i < *boatCount - 1; i++) {
        boats[i] = boats[i + 1];
    }
    (*boatCount)--;
    printf("Boat '%s' removed successfully.\n", boatName);
}

//--------------------------------------------------------------------------------------------------

// Function to process payment
void processPayment(Boat **boats, int boatCount) {
    char boatName[MAX_NAME_LENGTH];
    printf("Enter boat name to process payment: ");
    scanf(" %127[^\n]", boatName);
    for (int i = 0; i < boatCount; i++) {
        if (strcmp(boats[i]->BoatName, boatName) == 0) {
            double payment;
            printf("Enter payment amount: ");
            scanf("%lf", &payment);
            if (payment > boats[i]->AmountOwed) {
                printf("That is more than the amount owed, $%.2f\n", boats[i]->AmountOwed);
                return;  // Exit the function if the payment exceeds the amount owed
            }
            boats[i]->AmountOwed -= payment;
            if (boats[i]->AmountOwed < 0) {
                boats[i]->AmountOwed = 0;
            }
            printf("Payment processed. Remaining amount owed: $%.2f\n", boats[i]->AmountOwed);
            return;
        }
    }
    printf("Boat with name '%s' not found.\n", boatName);
}

//--------------------------------------------------------------------------------------------------

// Function for monthly updates
void monthlyUpdate(Boat **boats, int boatCount) {
   int i = 0;
     while( i < boatCount) {
        double monthlyCharge = 0.0;
        switch (boats[i]->TypeOfPlace) {
           case slip:
               monthlyCharge = boats[i]->BoatLength * 12.50;
               break;
           case land:
               monthlyCharge = boats[i]->BoatLength * 14;
               break;
           case trailor:
               monthlyCharge = boats[i]->BoatLength * 25;
               break;
           case storage:
               monthlyCharge = boats[i]->BoatLength * 11.20;
               break;
           }
    boats[i]->AmountOwed += monthlyCharge;
    i++;
    }
    
}

//--------------------------------------------------------------------------------------------------

// Main function

int main(int argc, char *argv[]) {
   if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Save the filename from argv[1] into a variable
    const char *filename = argv[1];
    // Display welcome message
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");
    Boat *boats[MAX_BOATS] = { NULL };
    int boatCount = loadBoatsFromCSV(filename, boats);
    if (boatCount == -1) {
        return EXIT_FAILURE;
    }
    // Main program loop
    char option;
    do {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &option);
        
        // Handle the input options
        switch (option) {
            case 'I':
            case 'i':
                // Display the boat inventory
                displayBoatInventory(boats, boatCount);
                break;
            case 'A':
            case 'a':
                // Add a boat
                addBoat(boats, &boatCount);
                break;
            case 'R':
            case 'r':
                // Remove a boat
                removeBoat(boats, &boatCount);
                break;
            case 'P':
            case 'p':
                // Process payment
                processPayment(boats, boatCount);
                break;
            case 'M':
            case 'm':
                // Monthly update
                monthlyUpdate(boats, boatCount);
                break;
            case 'X':
            case 'x':
                // Exit and save data
                saveBoatsToCSV(filename, boats, boatCount);
                printf("Exiting the Boat Management System\n");
                break;
            default:
                printf("Invalid option %c\n", option);
        }
    } while (option != 'x' && option != 'X');
    // Free dynamically allocated memory
    for (int i = 0; i < boatCount; i++) {
        free(boats[i]);
    }
    return EXIT_SUCCESS;
}



