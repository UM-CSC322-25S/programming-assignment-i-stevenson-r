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
        printf("%-20s %-6d %-8s ", boat->BoatName, boat->BoatLength,
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
        printf("$%.2f\n", boat->AmountOwed);
    }
}
//--------------------------------------------------------------------------------------------------
// Function to add a boat 
void addBoat(Boat **boats, int *boatCount) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }
    Boat newBoat;
    printf("Enter boat name: ");
    scanf(" %127[^\n]", newBoat.BoatName);
    printf("Enter boat length (in feet): ");
    scanf("%d", &newBoat.BoatLength);
    
    // Check if the boat length exceeds the limit
    if (newBoat.BoatLength > MAX_BOAT_LENGTH) {
        printf("Boat length exceeds the maximum allowed length of %d feet.\n", MAX_BOAT_LENGTH);
        return;
    }
    printf("Enter place type (slip, land, trailor, storage): ");
    char placeType[MAX_NAME_LENGTH];
    scanf(" %127[^\n]", placeType);
    if (strcmp(placeType, "slip") == 0) {
        newBoat.TypeOfPlace = slip;
        printf("Enter slip number: ");
        scanf("%d", &newBoat.PlaceInformation.SlipNumber);
    } else if (strcmp(placeType, "land") == 0) {
        newBoat.TypeOfPlace = land;
        printf("Enter bay letter: ");
        scanf(" %c", &newBoat.PlaceInformation.BayLetter);
    } else if (strcmp(placeType, "trailor") == 0) {
        newBoat.TypeOfPlace = trailor;
        printf("Enter trailor license tag: ");
        scanf(" %6s", newBoat.PlaceInformation.TrailorLicenseTag);
    } else if (strcmp(placeType, "storage") == 0) {
        newBoat.TypeOfPlace = storage;
        printf("Enter storage space number: ");
        scanf("%d", &newBoat.PlaceInformation.StorageSpaceNumber);
    } else {
        printf("Invalid place type.\n");
        return;
    }
    printf("Enter amount owed: ");
    scanf("%lf", &newBoat.AmountOwed);
    boats[*boatCount] = Malloc(sizeof(Boat));
    *boats[*boatCount] = newBoat;
    (*boatCount)++;
    printf("Boat added successfully.\n");
}

/*
void addBoat(Boat **boats, int *boatCount) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }
    Boat newBoat;
    printf("Please enter the boat data in CSV format : ");
    scanf("%s[^,", line);
        char placeType[MAX_NAME_LENGTH];
    char remainingData[MAX_NAME_LENGTH];
    sscanf(line, "%127[^,],%d", newBoat->BoatName, &newBoat->BoatLength);
    char *remainingDataPtr = strchr(line, ',');
    remainingDataPtr++;
    remainingDataPtr = strchr(remainingDataPtr, ',');
    remainingDataPtr++;
    sscanf(remainingDataPtr, "%127[^,]", placeType);
    if (strcmp(placeType, "slip") == 0) {
        newBoat->TypeOfPlace = slip;
        // Now parse the slip number after the place type
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        sscanf(remainingDataPtr, "%d", &newBoat->PlaceInformation.SlipNumber);
    } else if (strcmp(placeType, "land") == 0) {
        newBoat->TypeOfPlace = land;
        // Now parse the bay letter for land
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        sscanf(remainingDataPtr, "%c", &newBoat->PlaceInformation.BayLetter) != 1) 
        printf("Bay Letter: %c\n", newBoat->PlaceInformation.BayLetter);
    } else if (strcmp(placeType, "trailor") == 0) {
        newBoat->TypeOfPlace = trailor;
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  
        sscanf(remainingDataPtr, "%6s", newBoat->PlaceInformation.TrailorLicenseTag);
    } else if (strcmp(placeType, "storage") == 0) {
        newBoat->TypeOfPlace = storage;
        // Now parse the storage space number
        remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place type)
        sscanf(remainingDataPtr, "%d", &newBoat->PlaceInformation.StorageSpaceNumber);
    } else {
        printf("Unknown place type: %s\n", placeType);
    }
    // After determining the place type and parsing the specific place information, parse the AmountOwed
    remainingDataPtr = strchr(remainingDataPtr, ',') + 1;  // Move to the next comma (skip place information)
    sscanf(remainingDataPtr, "%lf", &newBoat->AmountOwed);
    //sscanf(line, "%127[^,],%d,%127[^,],%d,%lf", 
                            newBoat->BoatName, 
                            &newBoat->BoatLength, 
                            placeType, 
                            &newBoat->PlaceInformation.SlipNumber, 
                            &newBoat->AmountOwed);

    printf("Enter boat name: ");
    scanf(" %127[^\n]", newBoat.BoatName);
    printf("Enter boat length (in feet): ");
    scanf("%d", &newBoat.BoatLength);
    
    if (newBoat.BoatLength > MAX_BOAT_LENGTH) {
        printf("Boat length exceeds the maximum allowed length of %d feet.\n", MAX_BOAT_LENGTH);
        return;
    }
    printf("Enter place type (slip, land, trailor, storage): ");
    char placeType[MAX_NAME_LENGTH];
    scanf(" %127[^\n]", placeType);
    if (strcmp(placeType, "slip") == 0) {
        newBoat.TypeOfPlace = slip;
        printf("Enter slip number: ");
        scanf("%d", &newBoat.PlaceInformation.SlipNumber);
    } else if (strcmp(placeType, "land") == 0) {
        newBoat.TypeOfPlace = land;
        printf("Enter bay letter: ");
        scanf(" %c", &newBoat.PlaceInformation.BayLetter);
    } else if (strcmp(placeType, "trailor") == 0) {
        newBoat.TypeOfPlace = trailor;
        printf("Enter trailor license tag: ");
        scanf(" %6s", newBoat.PlaceInformation.TrailorLicenseTag);
    } else if (strcmp(placeType, "storage") == 0) {
        newBoat.TypeOfPlace = storage;
        printf("Enter storage space number: ");
        scanf("%d", &newBoat.PlaceInformation.StorageSpaceNumber);
    } else {
        printf("Invalid place type.\n");
        return;
    }
    printf("Enter amount owed: ");
    scanf("%lf", &newBoat.AmountOwed);
    boats[*boatCount] = Malloc(sizeof(Boat));
    *boats[*boatCount] = newBoat;
}
*/
/*
// Function to add a boat by reading a CSV input string
void addBoat(Boat **boats, int *boatCount) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }
    // Prompt for the CSV input string (all fields in one line)
    char inputLine[MAX_NAME_LENGTH + MAX_BOAT_LENGTH + 100]; // Max length for boat name, length, and place data
    printf("Enter boat details in CSV format (BoatName,BoatLength,PlaceType,PlaceSpecificInfo,AmountOwed): ");
    scanf(" %[^\n]", inputLine);  // Read the whole line as input
    Boat newBoat;
    char placeType[MAX_NAME_LENGTH];
    int itemsRead = sscanf(inputLine, "%127[^,],%d,%127[^,],%127[^\n],%lf", 
                            newBoat.BoatName, 
                            &newBoat.BoatLength, 
                            placeType, 
                            placeType,  // place specific info
                            &newBoat.AmountOwed);
    
    // Ensure boat length does not exceed the max allowed
    if (newBoat.BoatLength > MAX_BOAT_LENGTH) {
        printf("Boat length exceeds the maximum allowed length of %d feet.\n", MAX_BOAT_LENGTH);
        return;
    }
    if (itemsRead == 5) {
        // Handle the place type and specific info
        if (strcmp(placeType, "slip") == 0) {
            newBoat.TypeOfPlace = slip;
            sscanf(placeType, "%d", &newBoat.PlaceInformation.SlipNumber);
        } else if (strcmp(placeType, "land") == 0) {
            newBoat.TypeOfPlace = land;
            sscanf(placeType, "%c", &newBoat.PlaceInformation.BayLetter);
        } else if (strcmp(placeType, "trailor") == 0) {  // Changed back to 'trailor'
            newBoat.TypeOfPlace = trailor;
            sscanf(placeType, "%s", newBoat.PlaceInformation.TrailorLicenseTag);  // 7 characters max
        } else if (strcmp(placeType, "storage") == 0) {
            newBoat.TypeOfPlace = storage;
            sscanf(placeType, "%d", &newBoat.PlaceInformation.StorageSpaceNumber);
        } else {
            printf("Invalid place type.\n");
            return;
        }
        // Add the new boat to the array
        boats[*boatCount] = Malloc(sizeof(Boat));
        *boats[*boatCount] = newBoat;
        (*boatCount)++;
        printf("Boat added successfully.\n");
    } else {
        printf("Invalid CSV format.\n");
    }
}
// Function to add a boat by reading a CSV input string
void addBoat(Boat **boats, int *boatCount) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }
    // Prompt for the CSV input string (all fields in one line)
    char inputLine[MAX_NAME_LENGTH + MAX_BOAT_LENGTH + 100]; // Max length for boat name, length, and place data
    printf("Enter boat details in CSV format (BoatName,BoatLength,PlaceType,PlaceSpecificInfo,AmountOwed): ");
    scanf(" %[^\n]", inputLine);  // Read the whole line as input
    Boat newBoat;
    char placeType[MAX_NAME_LENGTH];
    char placeSpecificInfo[MAX_NAME_LENGTH]; // For place-specific info (like slip number, bay letter, etc.)
    
    // Parse the input string
    int itemsRead = sscanf(inputLine, "%127[^,],%d,%127[^,],%127[^\n],%lf", 
                            newBoat.BoatName, 
                            &newBoat.BoatLength, 
                            placeType, 
                            placeSpecificInfo,  // place specific info
                            &newBoat.AmountOwed);
    // Ensure boat length does not exceed the max allowed
    if (newBoat.BoatLength > MAX_BOAT_LENGTH) {
        printf("Boat length exceeds the maximum allowed length of %d feet.\n", MAX_BOAT_LENGTH);
        return;
    }
    if (itemsRead == 5) {
        // Handle the place type and specific info
        if (strcmp(placeType, "slip") == 0) {
            newBoat.TypeOfPlace = slip;
            sscanf(placeSpecificInfo, "%d", &newBoat.PlaceInformation.SlipNumber);
        } else if (strcmp(placeType, "land") == 0) {
            newBoat.TypeOfPlace = land;
            sscanf(placeSpecificInfo, "%c", &newBoat.PlaceInformation.BayLetter);
        } else if (strcmp(placeType, "trailor") == 0) {
            newBoat.TypeOfPlace = trailor;
            sscanf(placeSpecificInfo, "%s", newBoat.PlaceInformation.TrailorLicenseTag); // 7 characters max
        } else if (strcmp(placeType, "storage") == 0) {
            newBoat.TypeOfPlace = storage;
            sscanf(placeSpecificInfo, "%d", &newBoat.PlaceInformation.StorageSpaceNumber);
        } else {
            printf("Invalid place type.\n");
            return;
        }
        // Add the new boat to the array
        boats[*boatCount] = malloc(sizeof(Boat));
        if (boats[*boatCount] == NULL) {
            printf("Memory allocation failed.\n");
            return;
        }
        *boats[*boatCount] = newBoat;
        (*boatCount)++;
        printf("Boat added successfully.\n");
    } else {
        printf("Invalid CSV format.\n");
    }
}
*/
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
   // double increasePercentage;
   // printf("Enter the percentage increase in amount owed (e.g., 10 for 10%%): ");
   // scanf("%lf", &increasePercentage);
   int i = 0;
     while( i < boatCount) {
        switch (boats[i]->TypeOfPlace) {
           case slip:
               boats[i]->AmountOwed += 12.50;
               break;
           case land:
               boats[i]->AmountOwed += 14;
               break;
           case trailor:
               boats[i]->AmountOwed += 25;
               break;
           case storage:
               boats[i]->AmountOwed += 11.20;
               break;
           }
    //    boats[i]->AmountOwed += boats[i]->AmountOwed * (increasePercentage / 100);
    i++;
    }
    
   // printf("Monthly update applied. All amounts owed have been increased by %.2f%%.\n", increasePercentage);
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


