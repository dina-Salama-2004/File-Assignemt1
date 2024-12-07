#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <strstream>
#include <iomanip>

using namespace std;
int count_id = 0;
// Rana part   primary index
void Delete(char id[]);
int getRecordCount(const char *filename, int recordSize)
{
    fstream file(filename, ios::binary | ios::in);
    if (!file.is_open())
    {
        cout << "File not found: " << filename << endl;
        return 0;
    }
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.close();
    return fileSize / recordSize;
}
void Insert_Appointment_ID_Sorted(char id[], short offset, int &count_id)
{
    fstream primary("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

    // Convert the Appointment ID from char array to integer
    int New_Appointment_id = 0;
    for (int i = 0; id[i] != '\0'; i++)
    {
        New_Appointment_id *= 10;
        New_Appointment_id += (id[i] - '0');
    }

    int existing_id = 0;
    short off = 0;
    bool find_position = false;

    if (count_id == 0)
    {
        // If this is the first record, directly insert it
        primary.write((char *)&New_Appointment_id, sizeof(New_Appointment_id));
        primary.write((char *)&offset, sizeof(offset));
        count_id++;
    }
    else
    {
        // Locate the insertion position
        primary.read((char *)&existing_id, sizeof(existing_id));
        while (primary.good())
        {
            if (existing_id > New_Appointment_id)
            {
                find_position = true;
                primary.seekg(-sizeof(existing_id), ios::cur); // Step back to insertion point
                off = primary.tellg();
                break;
            }
            primary.seekg(sizeof(short), ios::cur); // Skip offset
            primary.read((char *)&existing_id, sizeof(existing_id));
        }
        primary.close();
        primary.open("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

        if (!find_position)
        {
            // Insert at the end if no suitable position was found
            primary.seekg(count_id * (sizeof(int) + sizeof(short)), ios::beg);
            primary.write((char *)&New_Appointment_id, sizeof(int));
            primary.write((char *)&offset, sizeof(short));
            count_id++;
        }
        else
        {
            // Shift records to make space for the new record
            primary.seekg((count_id - 1) * (sizeof(int) + sizeof(short)), ios::beg);
            int endNum;
            short endOf;
            primary.read((char *)&endNum, sizeof(endNum));
            primary.read((char *)&endOf, sizeof(endOf));
            primary.seekg(off);

            while (primary.good())
            {
                int num1;
                short num1_Of;
                int num2;
                short num2_Of;

                primary.read((char *)&num1, sizeof(num1));
                primary.read((char *)&num1_Of, sizeof(num1_Of));

                primary.read((char *)&num2, sizeof(num2));
                primary.read((char *)&num2_Of, sizeof(num2_Of));

                primary.seekg(-2 * (sizeof(int) + sizeof(short)), ios::cur);
                primary.write((char *)&num1, sizeof(num1));
                primary.write((char *)&num1_Of, sizeof(num1_Of));
            }

            primary.close();
            primary.open("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);
            primary.seekg(0, ios::end);
            primary.write((char *)&endNum, sizeof(endNum));
            primary.write((char *)&endOf, sizeof(endOf));
            primary.seekg(off);
            primary.write((char *)&New_Appointment_id, sizeof(New_Appointment_id));
            primary.write((char *)&offset, sizeof(off));
            count_id++;
        }
    }

    primary.close();
}

class Appointment
{
public:
    char Appointment_ID[15];
    char Appointment_Date[30];
    char Doctor_ID[15];

    const static int maxRecordSize = 1000;

    void writeAppointment(fstream &file, Appointment &s)
    {
        char record[maxRecordSize];
        strcpy(record, s.Appointment_ID);
        strcat(record, "|");
        strcat(record, s.Appointment_Date);
        strcat(record, "|");
        strcat(record, s.Doctor_ID);
        strcat(record, "|");

        short length = strlen(record);

        file.write((char *)&length, sizeof(length));
        file.write(record, length);
    }

    void readAppointment(fstream &file, Appointment &s)
    {
        short length;
        file.read((char *)&length, sizeof(length));
        char *record = new char[length];
        file.read(record, length);

        istrstream strbuff(record);
        strbuff >> s;
        delete[] record;
    }

    friend istream &operator>>(istream &file, Appointment &s)
    {
        file.getline(s.Appointment_ID, 15, '|');
        file.getline(s.Appointment_Date, 30, '|');
        file.getline(s.Doctor_ID, 15, '|');
        return file;
    }
};

// secodary index on doctor id in appointement

void displaySecondary_DoctorId()
{
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in);
    if (!DoctorSec.is_open())
    {
        cout << "Secondary Index file not found.\n";
        return;
    }

    char doctorID[15];
    short linkedListPointer;

    while (DoctorSec.read(doctorID, 15))
    {
        DoctorSec.read((char *)&linkedListPointer, sizeof(linkedListPointer));
        cout << "Doctor ID: " << doctorID << ", Linked List Pointer: " << linkedListPointer << endl;
    }
    DoctorSec.close();
}
void addToLinkedList_DoctorId(const char *Doctor_ID, const char *Appointment_ID)
{
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    if (!DoctorLink.is_open())
    {
        cout << "Error opening linked list file." << endl;
        return;
    }
    DoctorLink.seekg(0, ios::end);
    DoctorLink.write(Doctor_ID, 15);
    DoctorLink.write(Appointment_ID, 15);
    short nega = -1;
    DoctorLink.write((char *)&nega, sizeof(nega));
    DoctorLink.close();
}
void insert_secondary_DoctorId(char Doctor_ID[15], char Appointment_ID[15])
{
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in | ios::out);

    char temp[15];
    short linkedListPointer;
    int totalEntries = getRecordCount("SecondaryIndexForDoctor.txt", 17);

    // Binary search for the doctor ID
    bool found = false;
    int insertPosition = totalEntries;
    for (int i = 0; i < totalEntries; i++)
    {
        DoctorSec.seekg(i * 17, ios::beg);
        DoctorSec.read(temp, 15);
        if (strcmp(temp, Doctor_ID) > 0)
        {
            insertPosition = i;
            found = false;
            break;
        }
        else if (strcmp(temp, Doctor_ID) == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        // Shift records to the right to make room for the new record
        for (int i = totalEntries - 1; i >= insertPosition; i--)
        {
            DoctorSec.seekg(i * 17, ios::beg);
            char existingDoctorID[15];
            short existingPointer;
            DoctorSec.read(existingDoctorID, 15);
            DoctorSec.read((char *)&existingPointer, sizeof(existingPointer));

            DoctorSec.seekp((i + 1) * 17, ios::beg);
            DoctorSec.write(existingDoctorID, 15);
            DoctorSec.write((char *)&existingPointer, sizeof(existingPointer));
        }

        // Insert the new Doctor ID and pointer at the correct position
        DoctorSec.seekp(insertPosition * 17, ios::beg);
        DoctorSec.write(Doctor_ID, 15);
        linkedListPointer = getRecordCount("LLIndexForDoctor.txt", 32);
        DoctorSec.write((char *)&linkedListPointer, sizeof(linkedListPointer));

        // Add the new appointment to the linked list
        addToLinkedList_DoctorId(Doctor_ID, Appointment_ID);
    }
    else
    {

        // Doctor ID exists, update the linked list
        DoctorLink.seekg(0, ios::end);
        int fileSize = DoctorLink.tellg();
        int entrySize = 32; // Size of each record (doctorID + appointmentID + nextPointer)
        int recordCount = fileSize / entrySize;

        for (int i = recordCount - 1; i >= 0; i--)
        {
            DoctorLink.seekg(i * entrySize, ios::beg);
            char doctorID[15];
            char appointmentID[15];
            short nextPointer;

            DoctorLink.read(doctorID, 15);
            DoctorLink.read(appointmentID, 15);
            DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

            if (strcmp(doctorID, Doctor_ID) == 0)
            {
                short newPointer = recordCount; // Update to the next appointment record
                DoctorLink.seekp(i * entrySize + 30, ios::beg);
                DoctorLink.write((char *)&newPointer, sizeof(newPointer));
                break;
            }
        }

        // Add the appointment to the linked list
        addToLinkedList_DoctorId(Doctor_ID, Appointment_ID);
    }

    DoctorSec.close();
    DoctorLink.close();
}

void displayLinkedList_DoctorId()
{
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in);
    if (!DoctorLink.is_open())
    {
        cout << "Linked List file not found.\n";
        return;
    }

    char doctorID[15];
    char appointmentID[15];
    short nextPointer;

    // Read the content of the linked list file
    while (DoctorLink.read(doctorID, 15))
    {
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

        cout << "Doctor ID: " << doctorID
             << ", Appointment ID: " << appointmentID
             << ", Next Pointer: " << nextPointer << endl;
    }
    DoctorLink.close();
}
void write_Number_of_appointment()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Open the file to append data and also read it to determine the current number of records
    fstream file("Appointment.txt", ios::in | ios::out | ios::binary | ios::ate);
    int count_id = 0;

    // Calculate the current number of records if the file is not empty
    if (file.tellg() > 0)
    {
        file.seekg(0, ios::end);
        count_id = file.tellg() / sizeof(Appointment); // Assuming Appointment size is fixed
    }

    cout << "Enter # of Appointment you want to enter" << endl;
    int count;
    cin >> count;
    cin.ignore();

    Appointment record;

    // Loop to take input for new records
    for (int i = 0; i < count; i++)
    {
        cout << "ID: " << endl;
        cin >> record.Appointment_ID;
        cout << "Date:" << endl;
        cin.ignore();
        cin.getline(record.Appointment_Date, 30);
        cout << "Doctor ID: " << endl;
        cin >> record.Doctor_ID;

        // Get current write position for offset
        file.seekp(0, ios::end);
        cout << "Offset = " << file.tellp() << endl;

        // Insert into primary and secondary indices
        Insert_Appointment_ID_Sorted(record.Appointment_ID, file.tellp(), count_id);
        insert_secondary_DoctorId(record.Doctor_ID, record.Appointment_ID);

        // Write the new record to the file
        record.writeAppointment(file, record);
    }

    file.close();

    // Open the file again to display its contents
    file.open("Appointment.txt", ios::in | ios::binary);
    Appointment s2;

    cout << "Content of appointment file that inserted:" << "\n";
    for (int i = 0; i < count_id; i++)
    {
        cout << "Record " << i + 1 << endl;
        s2.readAppointment(file, s2);
        cout << "ID: " << s2.Appointment_ID << endl;
        cout << "Date: " << s2.Appointment_Date << endl;
        cout << "Doctor ID: " << s2.Doctor_ID << endl;
        cout << "-------------------------\n";
    }

    file.close();
}

// search

/*short getOffsetFromPrimaryIndex(const char* filename, char id[15]) {
    fstream primary(filename, ios::binary | ios::in);


    //convert from char to int
    int ID = 0;
    for (int i = 0; id[i] != '\0'; i++) {
        ID *= 10;
        ID += (id[i] - '0');
    }

    int existing_id = 0;
    short offset = -1;

    while (primary.read((char*)&existing_id, sizeof(existing_id))) {
        primary.read((char*)&offset, sizeof(offset));


        if (existing_id == ID) {
            primary.close();
            return offset;
        }
    }

    primary.close();
    return -1;
}*/

/*short getOffsetFromPrimaryIndex(const char *filename, const char id[15])
{
    fstream primary(filename, ios::binary | ios::in);
    if (!primary.is_open()) {
        cerr << "Error: Unable to open primary index file.\n";
        return -1;
    }

    char existing_id[15];  // Buffer to store the Doctor_ID from the file
    short offset = -1;     // Offset to return if found

    while (primary.read(existing_id, sizeof(existing_id))) {
        primary.read((char *)&offset, sizeof(offset)); // Read the associated offset

        if (strncmp(existing_id, id, 15) == 0) { // Compare the input ID with the one read
            primary.close();
            return offset;
        }
    }

    primary.close();
    return -1; // Return -1 if ID is not found
}*/
#include <iostream>
#include <fstream>
#include <cstring>

short getOffsetFromPrimaryIndex(const char *filename, const char id[15]) {
    std::fstream primary(filename, std::ios::binary | std::ios::in);
    if (!primary.is_open()) {
        std::cerr << "Error: Unable to open primary index file.\n";
        return -1;
    }

    char existing_id[15];  // Buffer to store the Doctor_ID from the file
    short offset = -1;     // Offset to return if found

    // Read until the end of the file
    while (primary.read(existing_id, sizeof(existing_id))) {
        primary.read(reinterpret_cast<char*>(&offset), sizeof(offset)); // Read the associated offset

        // Check if the ID matches
        if (strncmp(existing_id, id, sizeof(existing_id)) == 0) {
            primary.close();
            return offset; // Return the found offset
        }
    }

    primary.close(); // Close the file
    return -1; // Return -1 if ID is not found
}


void searchAppointmentsByDoctorID(const char *Doctor_ID)
{
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in);
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in);
    fstream AppointmentFile("Appointment.txt", ios::binary | ios::in);

    if (!DoctorSec.is_open() || !DoctorLink.is_open() || !AppointmentFile.is_open())
    {
        cout << "Error: Unable to open one or more files.\n";
        return;
    }

    // search in the secondary index to check if this id exist   and keep linkedListPointer that return first pointer to linked list
    short first = 0, last = getRecordCount("SecondaryIndexForDoctor.txt", 17) - 1;
    char tempDoctorID[15];
    short linkedListPointer = -1;
    bool found = false;

    while (first <= last)
    {
        short mid = (first + last) / 2;
        DoctorSec.seekg(mid * 17, ios::beg);
        DoctorSec.read(tempDoctorID, 15);

        int cmpResult = strcmp(tempDoctorID, Doctor_ID);
        if (cmpResult == 0)
        {
            DoctorSec.read((char *)&linkedListPointer, sizeof(linkedListPointer));
            found = true;
            break;
        }
        else if (cmpResult > 0)
        {
            last = mid - 1;
        }
        else
        {
            first = mid + 1;
        }
    }

    if (!found)
    {
        cout << "Doctor ID not found in the secondary index.\n";
        DoctorSec.close();
        DoctorLink.close();
        AppointmentFile.close();
        return;
    }

    // loop in linked list  while pointer !=-1
    cout << "Appointments for Doctor ID: " << Doctor_ID << endl;
    while (linkedListPointer != -1)
    {
        DoctorLink.seekg(linkedListPointer * 32, ios::beg);

        char doctorID[15];
        char appointmentID[15];
        short nextPointer;

        DoctorLink.read(doctorID, 15);
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

        if (strcmp(doctorID, Doctor_ID) != 0)
        {
            cout << "Error: Mismatched doctor ID in linked list.\n";
            break;
        }

        // get offset appointment ID from the primary index
        short offset = getOffsetFromPrimaryIndex("appointment_ID_primary_Index.txt", appointmentID);

        if (offset == -1)
        {
            cout << "Error: Appointment ID " << appointmentID << " not found in primary index.\n";
        }
        else
        {

            AppointmentFile.seekg(offset, ios::beg);
            short length;
            AppointmentFile.read((char *)&length, sizeof(length));

            char *record = new char[length];
            AppointmentFile.read(record, length);

            // Convert the Data to an Appointment Object
            istrstream strbuff(record);
            Appointment s;
            strbuff >> s;
            delete[] record;

            cout << "- Appointment ID: " << s.Appointment_ID
                 << ", Date: " << s.Appointment_Date
                 << ", Doctor ID: " << s.Doctor_ID << endl;
        }

        linkedListPointer = nextPointer;
    }

    DoctorSec.close();
    DoctorLink.close();
    AppointmentFile.close();
}

void testSearch_DI()
{
    char doctorID[15];
    cout << "Enter Doctor ID to search: ";

    cin >> setw(15) >> doctorID; // Limit input to 15 characters to prevent buffer overflow

    // Test case
    cout << "Running test for Doctor ID: " << doctorID << endl;
    searchAppointmentsByDoctorID(doctorID);
    cout << "Test complete.\n";
}

//-----------------------------------------------doctor part

class Doctor
{
public:
    char Doctor_ID[15];
    char Doctor_Name[30];
    char Address[30];

    const static int maxRecordSize = 1000;

    void writeDoctor(fstream &file, Doctor &d)
    {
        char record[maxRecordSize];
        strcpy(record, d.Doctor_ID);
        strcat(record, "|");
        strcat(record, d.Doctor_Name);
        strcat(record, "|");
        strcat(record, d.Address);
        strcat(record, "|");

        short length = strlen(record);

        file.write((char *)&length, sizeof(length));
        file.write(record, length);
    }

    void readDoctor(fstream &file, Doctor &d)
    {
        short length;
        file.read((char *)&length, sizeof(length));
        char *record = new char[length];
        file.read(record, length);

        istrstream strbuff(record);
        strbuff >> d;
        delete[] record;
    }

    friend istream &operator>>(istream &file, Doctor &d)
    {
        file.getline(d.Doctor_ID, 15, '|');
        file.getline(d.Doctor_Name, 30, '|');
        file.getline(d.Address, 30, '|');
        return file;
    }

    friend ostream &operator<<(ostream &os, const Doctor &d)
    {
        os << "Doctor ID: " << d.Doctor_ID << endl;
        os << "Doctor Name: " << d.Doctor_Name << endl;
        os << "Address: " << d.Address << endl;
        return os;
    }
};

void Insert_Doctor_ID_Sorted(char id[], short offset)
{
    fstream primary("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

    // Convert the Doctor ID from char array to a comparable integer-like value
    long long New_Doctor_id = 0;
    for (int i = 0; id[i] != '\0'; i++)
    {
        New_Doctor_id *= 256;   // Using base 256 to handle the full character set
        New_Doctor_id += id[i]; // Add each character's ASCII value
    }

    long long existing_id = 0;
    short off = 0;
    bool find_position = false;

    if (count_id == 0)
    {
        // If this is the first record, directly insert it
        primary.write((char *)&New_Doctor_id, sizeof(New_Doctor_id));
        primary.write((char *)&offset, sizeof(offset));
        count_id++;
    }
    else
    {
        // Locate the insertion position
        primary.read((char *)&existing_id, sizeof(existing_id));
        while (primary.good())
        {
            if (existing_id > New_Doctor_id)
            {
                find_position = true;
                primary.seekg(-sizeof(existing_id), ios::cur); // Step back to insertion point
                off = primary.tellg();
                break;
            }
            primary.seekg(sizeof(short), ios::cur); // Skip offset
            primary.read((char *)&existing_id, sizeof(existing_id));
        }
        primary.close();
        primary.open("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

        if (!find_position)
        {
            // Insert at the end if no suitable position was found
            primary.seekg(count_id * (sizeof(long long) + sizeof(short)), ios::beg);
            primary.write((char *)&New_Doctor_id, sizeof(long long));
            primary.write((char *)&offset, sizeof(short));
            count_id++;
        }
        else
        {
            // Shift records to make space for the new record
            primary.seekg((count_id - 1) * (sizeof(long long) + sizeof(short)), ios::beg);
            long long endNum;
            short endOf;
            primary.read((char *)&endNum, sizeof(endNum));
            primary.read((char *)&endOf, sizeof(endOf));
            primary.seekg(off);

            while (primary.good())
            {
                long long num1;
                short num1_Of;
                long long num2;
                short num2_Of;

                primary.read((char *)&num1, sizeof(num1));
                primary.read((char *)&num1_Of, sizeof(num1_Of));

                primary.read((char *)&num2, sizeof(num2));
                primary.read((char *)&num2_Of, sizeof(num2_Of));

                primary.seekg(-2 * (sizeof(long long) + sizeof(short)), ios::cur);
                primary.write((char *)&num1, sizeof(num1));
                primary.write((char *)&num1_Of, sizeof(num1_Of));
            }

            primary.close();
            primary.open("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);
            primary.seekg(0, ios::end);
            primary.write((char *)&endNum, sizeof(endNum));
            primary.write((char *)&endOf, sizeof(endOf));
            primary.seekg(off);
            primary.write((char *)&New_Doctor_id, sizeof(New_Doctor_id));
            primary.write((char *)&offset, sizeof(off));
            count_id++;
        }
    }

    primary.close();
}


void print_Doctor_ID_Primary_Index()
{
    fstream primary("doctor_ID_primary_Index.txt", ios::in | ios::binary);

    if (!primary)
    {
        std::cout << "Failed to open the primary index file." << std::endl;
        return;
    }

    long long doctor_id = 0;
    short offset = 0;

    std::cout << "Content of Doctor ID Primary Index:" << std::endl;
    int record_count = 1;
    while (primary.read((char *)&doctor_id, sizeof(doctor_id)) && primary.read((char *)&offset, sizeof(offset)))
    {
        std::cout << "Record " << record_count++ << ":" << std::endl;

        // Convert the doctor ID back to a string
        char doctor_id_str[16]; // Buffer for the Doctor ID
        int idx = 0;
        while (doctor_id > 0)
        {
            doctor_id_str[idx++] = static_cast<char>(doctor_id % 256); // Extract each byte
            doctor_id /= 256;
        }
        doctor_id_str[idx] = '\0';

        std::cout << "Doctor ID: " << doctor_id_str << std::endl;
        std::cout << "Offset: " << offset << std::endl;
        std::cout << "----------------------------" << std::endl;
    }

    if (primary.eof())
    {
        std::cout << "End of index file reached." << std::endl;
    }
    else
    {
        std::cout << "Error reading from the file." << std::endl;
    }

    primary.close();
}

//-------------------------------- seconfdary index

// Function to display the secondary index (Doctor_Name with pointer)
void displaySecondary_DoctorName()
{
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in);
    if (!DoctorSec.is_open())
    {
        cout << "Secondary Index file not found.\n";
        return;
    }

    char doctorName[30]; // Adjust size to match Doctor_Name length
    short linkedListPointer;

    while (DoctorSec.read(doctorName, 30))
    {
        DoctorSec.read((char *)&linkedListPointer, sizeof(linkedListPointer));
        cout << "Doctor Name: " << doctorName << ", Linked List Pointer: " << linkedListPointer << endl;
    }
    DoctorSec.close();
}

// Function to add a new record to the linked list for Doctor_Name
void addToLinkedList_DoctorName(const char *Doctor_Name, const char *Appointment_ID)
{
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in | ios::out);
    if (!DoctorLink.is_open())
    {
        cout << "Error opening linked list file." << endl;
        return;
    }
    DoctorLink.seekg(0, ios::end);
    DoctorLink.write(Doctor_Name, 30);             // Store the Doctor_Name
    DoctorLink.write(Appointment_ID, 15);          // Store the Appointment_ID
    short nega = -1;                               // End of linked list marker
    DoctorLink.write((char *)&nega, sizeof(nega)); // Pointer to next (initially -1)
    DoctorLink.close();
}

// Function to insert a new Doctor_Name with pointer to the linked list in the secondary index
void insert_secondary_DoctorName(char Doctor_Name[30], char Appointment_ID[15])
{
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in | ios::out);
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in | ios::out);

    char temp[30];
    short linkedListPointer;
    int totalEntries = getRecordCount("SecondaryIndexForDoctorName.txt", 32); // 30 + 2 (pointer size)

    // Binary search for the Doctor Name in the secondary index
    bool found = false;
    int insertPosition = totalEntries;
    for (int i = 0; i < totalEntries; i++)
    {
        DoctorSec.seekg(i * 32, ios::beg); // Each entry is 32 bytes (30 for name, 2 for pointer)
        DoctorSec.read(temp, 30);
        if (strcmp(temp, Doctor_Name) > 0)
        {
            insertPosition = i;
            found = false;
            break;
        }
        else if (strcmp(temp, Doctor_Name) == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        // Shift records to the right to make room for the new record
        for (int i = totalEntries - 1; i >= insertPosition; i--)
        {
            DoctorSec.seekg(i * 32, ios::beg);
            char existingDoctorName[30];
            short existingPointer;
            DoctorSec.read(existingDoctorName, 30);
            DoctorSec.read((char *)&existingPointer, sizeof(existingPointer));

            DoctorSec.seekp((i + 1) * 32, ios::beg);
            DoctorSec.write(existingDoctorName, 30);
            DoctorSec.write((char *)&existingPointer, sizeof(existingPointer));
        }

        // Insert the new Doctor Name and pointer at the correct position
        DoctorSec.seekp(insertPosition * 32, ios::beg);
        DoctorSec.write(Doctor_Name, 30);
        linkedListPointer = getRecordCount("LLIndexForDoctorName.txt", 47); // Get the current position in linked list
        DoctorSec.write((char *)&linkedListPointer, sizeof(linkedListPointer));

        // Add the new appointment to the linked list
        addToLinkedList_DoctorName(Doctor_Name, Appointment_ID);
    }
    else
    {
        // Doctor Name exists, update the linked list
        DoctorLink.seekg(0, ios::end);
        int fileSize = DoctorLink.tellg();
        int entrySize = 47; // Size of each record (DoctorName + AppointmentID + NextPointer)
        int recordCount = fileSize / entrySize;

        for (int i = recordCount - 1; i >= 0; i--)
        {
            DoctorLink.seekg(i * entrySize, ios::beg);
            char doctorName[30];
            char appointmentID[15];
            short nextPointer;

            DoctorLink.read(doctorName, 30);
            DoctorLink.read(appointmentID, 15);
            DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

            if (strcmp(doctorName, Doctor_Name) == 0)
            {
                short newPointer = recordCount; // Update to the next appointment record
                DoctorLink.seekp(i * entrySize + 45, ios::beg);
                DoctorLink.write((char *)&newPointer, sizeof(newPointer));
                break;
            }
        }

        // Add the appointment to the linked list
        addToLinkedList_DoctorName(Doctor_Name, Appointment_ID);
    }

    DoctorSec.close();
    DoctorLink.close();
}

// Function to display the linked list of Doctor Names with appointments
void displayLinkedList_DoctorName()
{
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in);
    if (!DoctorLink.is_open())
    {
        cout << "Linked List file not found.\n";
        return;
    }

    char doctorName[30]; // Adjust to match Doctor_Name length
    char appointmentID[15];
    short nextPointer;

    // Read the content of the linked list file
    while (DoctorLink.read(doctorName, 30))
    {
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

        cout << "Doctor Name: " << doctorName
             << ", Appointment ID: " << appointmentID
             << ", Next Pointer: " << nextPointer << endl;
    }
    DoctorLink.close();
}

//-------------- serch

void searchAppointmentsByDoctorName(const char *Doctor_Name)
{
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in);
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in);
    fstream AppointmentFile("Appointment.txt", ios::binary | ios::in);

    if (!DoctorSec.is_open() || !DoctorLink.is_open() || !AppointmentFile.is_open())
    {
        cout << "Error: Unable to open one or more files.\n";
        return;
    }

    // Search in the secondary index to check if this name exists, and keep linkedListPointer that returns the first pointer to the linked list
    short first = 0, last = getRecordCount("SecondaryIndexForDoctorName.txt", 32) - 1; // 32 for DoctorName (30) + pointer (2)
    char tempDoctorName[30];                                                           // Assuming Doctor_Name is of length 30
    short linkedListPointer = -1;
    bool found = false;

    while (first <= last)
    {
        short mid = (first + last) / 2;
        DoctorSec.seekg(mid * 32, ios::beg); // Each entry is 32 bytes
        DoctorSec.read(tempDoctorName, 30);

        int cmpResult = strcmp(tempDoctorName, Doctor_Name);
        if (cmpResult == 0)
        {
            DoctorSec.read((char *)&linkedListPointer, sizeof(linkedListPointer));
            found = true;
            break;
        }
        else if (cmpResult > 0)
        {
            last = mid - 1;
        }
        else
        {
            first = mid + 1;
        }
    }

    if (!found)
    {
        cout << "Doctor Name not found in the secondary index.\n";
        DoctorSec.close();
        DoctorLink.close();
        AppointmentFile.close();
        return;
    }

    // Loop through linked list while pointer != -1
    cout << "Appointments for Doctor Name: " << Doctor_Name << endl;
    while (linkedListPointer != -1)
    {
        DoctorLink.seekg(linkedListPointer * 47, ios::beg); // 47 bytes per record (DoctorName + AppointmentID + NextPointer)

        char doctorName[30];
        char appointmentID[15];
        short nextPointer;

        DoctorLink.read(doctorName, 30);
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char *)&nextPointer, sizeof(nextPointer));

        if (strcmp(doctorName, Doctor_Name) != 0)
        {
            cout << "Error: Mismatched doctor name in linked list.\n";
            break;
        }

        // Get offset for appointment ID from the primary index
        short offset = getOffsetFromPrimaryIndex("appointment_ID_primary_Index.txt", appointmentID);

        if (offset == -1)
        {
            cout << "Error: Appointment ID " << appointmentID << " not found in primary index.\n";
        }
        else
        {
            AppointmentFile.seekg(offset, ios::beg);
            short length;
            AppointmentFile.read((char *)&length, sizeof(length));

            char *record = new char[length];
            AppointmentFile.read(record, length);

            // Convert the data to an Appointment object
            istrstream strbuff(record);
            Appointment s;
            strbuff >> s;
            delete[] record;

            cout << "- Appointment ID: " << s.Appointment_ID
                 << ", Date: " << s.Appointment_Date;
        }

        linkedListPointer = nextPointer;
    }

    DoctorSec.close();
    DoctorLink.close();
    AppointmentFile.close();
}

// Test function for searching by doctor name
void testSearch_DN()
{
    char doctorName[30];
    cout << "Enter Doctor Name to search: ";

    cin >> setw(30) >> doctorName; // Limit input to 30 characters to prevent buffer overflow

    // Test case
    cout << "Running test for Doctor Name: " << doctorName << endl;
    searchAppointmentsByDoctorName(doctorName);
    cout << "Test complete.\n";
}

void add(Doctor doctor)
{
    fstream file("Doctor.txt", ios::out | ios::binary | ios ::in);

    short header;
    file.seekg(0, ios::beg);
    file.read((char *)&header, sizeof(header));
    short idSize = ::strlen(doctor.Doctor_ID);
    short nameSize = ::strlen(doctor.Doctor_Name);
    short addressSize = ::strlen(doctor.Address);
    short recordSize = idSize + nameSize + addressSize;
    ////update index file ///////////////////////////////
    /// update index file ///////////////////////////////
    insert_secondary_DoctorName(doctor.Doctor_Name, doctor.Doctor_ID);

    // check if avail list is empty or not
    if (header == -1)
    {
        file.seekp(0, ios::end);
        doctor.writeDoctor(file, doctor);
        short offset = file.tellp();
        //
        Insert_Doctor_ID_Sorted(doctor.Doctor_ID, offset);
        file.close();
    }
        // avail list is not empty so i have to check if there is any size fits new record
    else
    {
        // nextOffset stores offset of next available record in the file
        // deletedSize stores size of deleted record
        short nextOffset, deletedSize;
        file.seekg(header + 1, ios::beg);
        file.read((char *)&nextOffset, sizeof(nextOffset));
        file.seekg(header - 2, ios::beg);
        file.read((char *)&deletedSize, sizeof(deletedSize));

        short diff = deletedSize - recordSize;
        // add new record at the end because it does not fit
        if (diff < 0)
        {
            file.seekp(0, ios::end);
            doctor.writeDoctor(file, doctor);
            Insert_Doctor_ID_Sorted(doctor.Doctor_ID, file.tellp());
            ////update index file ///////////////////////////////
            ////update index file ///////////////////////////////
            file.close();
        }
        else
        {
            // When the available space is almost enough but has a small remainder
            // in this case diff added to record size
            if (diff - 2 < 5)
            {
                file.seekp(0, ios::beg);
                file.write((char *)&nextOffset, sizeof(nextOffset));
                file.seekp(header - 2, ios::beg);
                ////update index file ///////////////////////////////
                Insert_Doctor_ID_Sorted(doctor.Doctor_ID, header);

                recordSize += diff;
                doctor.writeDoctor(file, doctor);
                file.close();
            }
                // When the available space is significantly larger than required
                //
            else
            {
                file.seekp(0, ios::beg);
                file.write((char *)&nextOffset, sizeof(nextOffset));
                file.seekp(header - 2, ios::beg);
                ////update index file ///////////////////////////////
                Insert_Doctor_ID_Sorted(doctor.Doctor_ID, header);

                doctor.writeDoctor(file, doctor);

                file.seekg(-(::strlen(doctor.Doctor_ID) + ::strlen(doctor.Doctor_Name) + ::strlen(doctor.Address) + 10), ios::beg);
                file.seekg(recordSize, ios::cur);

                short fakeID = 1;
                diff -= 2;
                file.write((char *)&diff, sizeof(diff));

                file.write("#", 1);
                file.write("|", 1);
                file.write("1", 1);
                file.write("|", 1);
                file.close();
                char id[15] = "1";
                Delete(id);
            }
        }
    }
}

/*void Delete(char id[])
{
    short M = -1; //
    fstream file("Doctor.txt", ios::out | ios::binary | ios::in | ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Error opening file." << std::endl;
        return;
    }
    short offset = getOffsetFromPrimaryIndex("doctor_ID_primary_Index.txt", id); // -1 no record ,
    cout << offset <<"\n";
    short size;
    file.seekg(offset - 2, ios::beg);
    file.read((char *)&size, sizeof(size));

    // there is no id match the id you provide
    if (offset == -1)
    {
        cout << "Not Valid ID\n";
    }
        // search function return an offset
    else
    {
        file.seekg(0, ios::beg);
        short header;
        file.read((char *)&header, sizeof(header));
        cout << "header " << header << "\n";

        // avail list empty there is no delete records this is the first record to be deleted
        if (header == -1)
        {
            file.seekg(0, ios::beg);
            file.write((char *)&offset, sizeof(offset));
            file.seekg(offset, ios::beg);
            file.write("*", 1);
            file.write((char *)&M, sizeof(M));
            file.close();
        }
            // avail list is no empty contains at least one deleted record
        else
        {
            file.seekg(0, ios::beg);
            file.read((char *)&header, sizeof(header));
            short headerSize;
            file.seekg(header - 2, ios::beg);
            file.read((char *)&headerSize, sizeof(headerSize));
            if (size > headerSize)
            {
                file.seekg(0, ios::beg);
                file.write((char *)&offset, sizeof(offset));
                file.seekg(offset, ios::beg);
                file.write("*", 1);
                file.write((char *)&header, sizeof(header));
                file.close();
            }
            else
            {
                short prev, next, temp;
                prev = header;                    // start with first deleted record
                file.seekg(header + 1, ios::beg); //
                while (true)
                {
                    file.read((char *)&temp, sizeof(temp));
                    // we have reached end of avail list
                    if (temp == -1)
                    {
                        file.seekg(offset, ios::beg);
                        file.write("*", 1);
                        file.write((char *)&M, sizeof(M));           // set next to -1
                        file.seekg(prev + 1, ios::beg);              // update prev
                        file.write((char *)&offset, sizeof(offset)); // point to the deleted record
                        file.close();
                        break;
                    }
                    else
                    {
                        file.seekg(temp - 2, ios::beg);
                        file.read((char *)&next, sizeof(next));
                        if (size >= next)
                        {
                            file.seekg(offset, ios::beg);
                            file.write("*", 1);
                            file.write((char *)&temp, sizeof(temp));
                            file.seekg(prev + 1, ios::beg);              // update prev
                            file.write((char *)&offset, sizeof(offset)); // point to the deleted record
                            file.close();
                            break;
                        }
                        else
                        {
                            prev = temp;
                            file.seekg(prev + 1, ios::beg);
                        }
                    }
                }
                file.close();
            }
        }
    }
}*/

void Delete(char id[]) {
    short M = -1; // Marker for deleted records
    std::fstream file("Doctor.txt", std::ios::out | std::ios::binary | std::ios::in | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    short offset = getOffsetFromPrimaryIndex("doctor_ID_primary_Index.txt", id);
    std::cout << offset << "\n";

    if (offset == -1) {
        std::cout << "Not Valid ID\n";
        file.close();
        return;
    }

    file.seekg(offset - sizeof(short), std::ios::beg);
    short size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    file.seekg(0, std::ios::beg);
    short header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    std::cout << "header " << header << "\n";

    if (header == -1) {
        // First record to be deleted
        file.seekg(0, std::ios::beg);
        file.write(reinterpret_cast<char*>(&offset), sizeof(offset));
        file.seekg(offset, std::ios::beg);
        file.write("*", 1);
        file.write(reinterpret_cast<char*>(&M), sizeof(M));
    } else {
        // Handle the case where the available list is not empty
        short prev = header;
        short next;
        bool inserted = false;

        while (true) {
            file.seekg(prev + sizeof(short), std::ios::beg);
            file.read(reinterpret_cast<char*>(&next), sizeof(next));

            if (next == -1 || size < next) {
                // Insert the deleted record here
                file.seekg(offset, std::ios::beg);
                file.write("*", 1);
                file.write(reinterpret_cast<char*>(&next), sizeof(next));
                file.seekg(prev + sizeof(short), std::ios::beg);
                file.write(reinterpret_cast<char*>(&offset), sizeof(offset));
                inserted = true;
                break;
            }
            prev = next;
        }

        if (!inserted) {
            // If we reached the end of the list
            file.seekg(offset, std::ios::beg);
            file.write("*", 1);
            file.write(reinterpret_cast<char*>(&M), sizeof(M));
            file.seekg(prev + sizeof(short), std::ios::beg);
            file.write(reinterpret_cast<char*>(&offset), sizeof(offset));
        }
    }

    file.close();
}
void updateName(){
    fstream file("SecondaryIndexForDoctorName.txt", ios::in | ios::out | ios::binary);
    short beg = 0;
    short last;

}

void deletePrimary(int id){
    fstream file("doctor_ID_primary_Index.txt", ios::binary|ios::in|ios::out);
    short beg = 0;
    short last = count_id-1;
    short mid;
    bool found = false;
    int temp;
    while (beg <= last && !found){
        mid = (beg+last)/2;
        file.seekg(mid*6,ios::beg);
        file.read((char *)&temp, sizeof(temp));
        if (temp == id)
            found= true;
        else if (temp > id)
            last = mid-1;
        else
            beg = mid+1;
    }
    if (found){
        file.seekg((mid+1)*6, ios::beg);
        while (file.good()){
            int tmp;
            short off;
            file.read((char *)&tmp, sizeof(tmp));
            file.read((char *)&off, sizeof(off));
            file.seekg(-12, ios::cur);
            file.write((char *)&tmp, sizeof(tmp));
            file.write((char *)&off, sizeof(off));
            file.seekg(6, ios::cur);
        }
        file.close();
        fstream file("doctor_ID_primary_Index.txt", ios::binary|ios::in|ios::out);
        count_id--;
    }
    file.close();
}

int totalEntries = getRecordCount("SecondaryIndexForDoctorName.txt", 32); // 30 + 2 (pointer size)
void deleteSecondary(char name[30]){
    fstream file("SecondaryIndexForDoctorName.txt", ios::binary|ios::in|ios::out);
    short beg = 0;
    short last = count_id-1;
    short mid;
    bool found = false;
    char temp[30];
    while (beg <= last && !found){
        mid = (beg+last)/2;
        file.seekg(mid*34,ios::beg);
        file.read((char *)&temp, sizeof(temp));
        if (::strcmp(temp, name) == 0)
            found= true;
        else if (::strcmp(temp, name) == 1)
            last = mid-1;
        else
            beg = mid+1;
    }
    file.close();
    if (!found)
        return;
    file.open("SecondaryIndexForDoctorName.txt", ios::binary|ios::in|ios::out);

    file.seekg(((totalEntries-1)*34), ios::beg);
    file.seekg((mid+34), ios::beg);
    int i = mid/34;
    while (i < totalEntries-1){
        char tmp[30];
        short off;
        file.read(tmp, 5);
        file.read((char *)&off, sizeof(off));
        file.seekg(i*34, ios::beg);
        file.write(tmp, sizeof(tmp));
        file.write((char *)&off, sizeof(tmp));
        i++;
    }
    totalEntries--;
    file.close();


}
void Update(Doctor doctor)
{
    // extract doctor id as integer
    int t = 0;
    for (int i = 0; doctor.Doctor_ID[i] != '\0'; ++i)
    {
        t *= 10;
        t += (doctor.Doctor_ID[i] - '0');
    }
    deletePrimary(t);

    fstream file("Doctor.txt", ios::out | ios::in | ios::binary);
    short offset = getOffsetFromPrimaryIndex("doctor_ID_primary_Index.txt", doctor.Doctor_ID);
    if (offset == -1)
    {
        cout << "Doctor does not exist\n";
        file.close();
        return;
    }

    short recordSize;
    file.seekg(offset, ios::beg);
    file.read((char *)&recordSize, sizeof(recordSize));

    short idSize = ::strlen(doctor.Doctor_ID);
    short nameSize = ::strlen(doctor.Doctor_Name);
    short addressSize = ::strlen(doctor.Address);
    short newRecordSize = idSize + nameSize + addressSize;

    short difference = recordSize - newRecordSize;

    insert_secondary_DoctorName(doctor.Doctor_Name, doctor.Doctor_ID);
    // if the new record size grater than the original record
    if (difference < 0)
    {
        Delete(doctor.Doctor_ID);
        file.seekp(0, ios::end);
        Insert_Doctor_ID_Sorted(doctor.Doctor_ID, file.tellp());
    }
    else if (difference - 2 <= 5)
    {
        file.seekp(offset - 2, ios::beg);
        short off = file.tellp();
        Insert_Doctor_ID_Sorted(doctor.Doctor_ID, off+1);
    }
    else
    {
        file.seekp(offset-2, ios::beg);
        short off = file.tellp();
        Insert_Doctor_ID_Sorted(doctor.Doctor_ID, off+1);
    }
    file.write((char *)&newRecordSize, sizeof(newRecordSize));
    doctor.writeDoctor(file, doctor);
    file.close();
}

/*void Update(Doctor doctor) {
    std::fstream file("Doctor.txt", std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open Doctor.txt\n";
        return;
    }

    short offset = 31;
            //getOffsetFromPrimaryIndex("doctor_ID_primary_Index.txt", doctor.Doctor_ID);
    if (offset == -1) {
        std::cout << "Doctor does not exist\n";
        return;
    }

    short recordSize;
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&recordSize), sizeof(recordSize));

    short idSize = static_cast<short>(std::strlen(doctor.Doctor_ID));
    short nameSize = static_cast<short>(std::strlen(doctor.Doctor_Name));
    short addressSize = static_cast<short>(std::strlen(doctor.Address));
    short newRecordSize = idSize + nameSize + addressSize + sizeof(recordSize); // Include record size

    short difference = recordSize - newRecordSize;

    // If the new record size is greater than the original record
    if (difference < 0) {
        Delete(doctor.Doctor_ID);
        file.seekp(0, std::ios::end); // Move to the end of the file to append
    } else if (difference <= 5) { // Adjust this condition as needed
        file.seekp(offset - sizeof(short), std::ios::beg); // Update the record size
    } else {
        file.seekp(offset, std::ios::beg);
        file.write(reinterpret_cast<char*>(&newRecordSize), sizeof(newRecordSize)); // Update the record size
    }

    // Write the updated doctor record
    doctor.writeDoctor(file, doctor);
    file.close();
}*/

void write_Number_of_Doctor()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Open the file to append data and also read it to determine the current number of records
    fstream file("Doctor.txt", ios::in | ios::out | ios::binary | ios::ate);
    int count_id = 0;

    // Calculate the current number of records if the file is not empty
    if (file.tellg() > 0)
    {
        file.seekg(0, ios::end);
        count_id = file.tellg() / sizeof(Doctor); // Assuming Doctor size is fixed
    }

    cout << "Enter # of Doctors you want to enter" << endl;
    int count;
    cin >> count;
    cin.ignore();

    Doctor record;

    // Loop to take input for new records
    for (int i = 0; i < count; i++)
    {
        cout << "Doctor ID: " << endl;
        cin >> record.Doctor_ID;
        cout << "Doctor Name: " << endl;
        cin.ignore();
        cin.getline(record.Doctor_Name, 30);
        cout << "Address: " << endl;
        cin.getline(record.Address, 30);

        // Get current write position for offset
        file.seekp(0, ios::end);
        cout << "Offset = " << file.tellp() << endl;

        // Insert into primary and secondary indices
        //Insert_Doctor_ID_Sorted(record.Doctor_ID, file.tellp());
        //insert_secondary_DoctorName(record.Doctor_Name, record.Doctor_ID);

        // Write the new record to the file
        // record.writeDoctor(file, record);
        add(record);
//        Update(record);
    }

    file.close();

    // Open the file again to display its contents
    file.open("Doctor.txt", ios::in | ios::binary);
    Doctor s2;

    file.seekg(1,ios::beg);
    cout << "Content of doctor file that was inserted:" << "\n";
    for (int i = 0; i < count_id; i++)
    {
        cout << "Record " << i + 1 << endl;
        s2.readDoctor(file, s2);
        cout << "Doctor ID: " << s2.Doctor_ID << endl;
        cout << "Doctor Name: " << s2.Doctor_Name << endl;
        cout << "Address: " << s2.Address << endl;
        cout << "-------------------------\n";
    }

    file.close();
}
int main()
{

    fstream file("Doctor.txt", ios::in | ios::out | ios::binary);
    short h;
    file.seekg(0,ios::beg);
    file.read((char *)&h, sizeof(h));
    cout << "header " << h << "\n";
    file.close();
    short h = -1;
    file.write((char *)&h, sizeof(h));
    file.close();
    cout << " for doctor  \n";
    print_Doctor_ID_Primary_Index();
    write_Number_of_Doctor();
    displaySecondary_DoctorName();
    cout << "----------------------------\n";
    displayLinkedList_DoctorName();
//
//    cout << " for appointemnt \n";
//    // appointement part
//    // write_Number_of_appointment();
//    //
////    displaySecondary_DoctorId();
////    cout << "----------------------------\n";
////    displayLinkedList_DoctorId();
//
//    cout << "----------------------------\n";
//    //    testSearch_DN();
//    // testSearch_DI();

//
//    char doctorID[15];
//    cout << "Enter Doctor ID to delete: ";
//    cin >> doctorID;
//
//    // Call the Delete function
//    Delete(doctorID);



    return 0;
}