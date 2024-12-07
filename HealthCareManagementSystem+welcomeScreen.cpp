#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <strstream>
#include <iomanip>
using namespace std;
class Appointment {
public:
    char Appointment_ID[15];
    char Appointment_Date[30];
    char Doctor_ID[15];

    const static int maxRecordSize = 1000;

    void writeAppointment(fstream& file, Appointment& s) {
        char record[maxRecordSize];
        strcpy_s(record, s.Appointment_ID);
        strcat_s(record, "|");
        strcat_s(record, s.Appointment_Date);
        strcat_s(record, "|");
        strcat_s(record, s.Doctor_ID);
        strcat_s(record, "|");

        short length = strlen(record);

        file.write((char*)&length, sizeof(length));
        file.write(record, length);
    }

    void readAppointment(fstream& file, Appointment& s) {
        short length;
        file.read((char*)&length, sizeof(length));
        char* record = new char[length];
        file.read(record, length);

        istrstream strbuff(record);
        strbuff >> s;
        delete[] record;
    }

    friend istream& operator>>(istream& file, Appointment& s) {
        file.getline(s.Appointment_ID, 15, '|');
        file.getline(s.Appointment_Date, 30, '|');
        file.getline(s.Doctor_ID, 15, '|');
        return file;
    }
};

short count_id = 0;
void write_Number_of_appointment() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Open the file to append data and also read it to determine the current number of records
    fstream file("Appointment.txt", ios::in | ios::out | ios::binary | ios::ate);
    int count_id = 0;

    // Calculate the current number of records if the file is not empty
    if (file.tellg() > 0) {
        file.seekg(0, ios::end);
        count_id = file.tellg() / sizeof(Appointment); // Assuming Appointment size is fixed
    }

    cout << "Enter # of Appointment you want to enter" << endl;
    int count;
    cin >> count;
    cin.ignore();

    Appointment record;

    // Loop to take input for new records
    for (int i = 0; i < count; i++) {
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
        Insert_Appointment_ID_Sorted(record.Appointment_ID, file.tellp());
        insert_secondary_DoctorId(record.Doctor_ID, record.Appointment_ID);

        // Write the new record to the file
        record.writeAppointment(file, record);
    }

    file.close();

    // Open the file again to display its contents
    file.open("Appointment.txt", ios::in | ios::binary);
    Appointment s2;

    cout << "Content of appointment file that inserted:" << "\n";
    for (int i = 0; i < count_id; i++) {
        cout << "Record " << i + 1 << endl;
        s2.readAppointment(file, s2);
        cout << "ID: " << s2.Appointment_ID << endl;
        cout << "Date: " << s2.Appointment_Date << endl;
        cout << "Doctor ID: " << s2.Doctor_ID << endl;
        cout << "-------------------------\n";
    }

    file.close();
}
int getRecordCount(const char* filename, int recordSize) {
    fstream file(filename, ios::binary | ios::in);
    if (!file.is_open()) {
        cout << "File not found: " << filename << endl;
        return 0;
    }
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.close();
    return fileSize / recordSize;
}
void Insert_Appointment_ID_Sorted(char id[], short offset) {
    fstream primary("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

    // Convert the Appointment ID from char array to integer
    int New_Appointment_id = 0;
    for (int i = 0; id[i] != '\0'; i++) {
        New_Appointment_id *= 10;
        New_Appointment_id += (id[i] - '0');
    }

    int existing_id = 0;
    short off = 0;
    bool find_position = false;

    if (count_id == 0) {
        // If this is the first record, directly insert it
        primary.write((char*)&New_Appointment_id, sizeof(New_Appointment_id));
        primary.write((char*)&offset, sizeof(offset));
        count_id++;
    }
    else {
        // Locate the insertion position
        primary.read((char*)&existing_id, sizeof(existing_id));
        while (primary.good()) {
            if (existing_id > New_Appointment_id) {
                find_position = true;
                primary.seekg(-sizeof(existing_id), ios::cur); // Step back to insertion point
                off = primary.tellg();
                break;
            }
            primary.seekg(sizeof(short), ios::cur); // Skip offset
            primary.read((char*)&existing_id, sizeof(existing_id));
        }
        primary.close();
        primary.open("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

        if (!find_position) {
            // Insert at the end if no suitable position was found
            primary.seekg(count_id * (sizeof(int) + sizeof(short)), ios::beg);
            primary.write((char*)&New_Appointment_id, sizeof(int));
            primary.write((char*)&offset, sizeof(short));
            count_id++;
        }
        else {
            // Shift records to make space for the new record
            primary.seekg((count_id - 1) * (sizeof(int) + sizeof(short)), ios::beg);
            int endNum;
            short endOf;
            primary.read((char*)&endNum, sizeof(endNum));
            primary.read((char*)&endOf, sizeof(endOf));
            primary.seekg(off);

            while (primary.good()) {
                int num1;
                short num1_Of;
                int num2;
                short num2_Of;

                primary.read((char*)&num1, sizeof(num1));
                primary.read((char*)&num1_Of, sizeof(num1_Of));

                primary.read((char*)&num2, sizeof(num2));
                primary.read((char*)&num2_Of, sizeof(num2_Of));

                primary.seekg(-2 * (sizeof(int) + sizeof(short)), ios::cur);
                primary.write((char*)&num1, sizeof(num1));
                primary.write((char*)&num1_Of, sizeof(num1_Of));
            }

            primary.close();
            primary.open("appointment_ID_primary_Index.txt", ios::out | ios::binary | ios::in);
            primary.seekg(0, ios::end);
            primary.write((char*)&endNum, sizeof(endNum));
            primary.write((char*)&endOf, sizeof(endOf));
            primary.seekg(off);
            primary.write((char*)&New_Appointment_id, sizeof(New_Appointment_id));
            primary.write((char*)&offset, sizeof(off));
            count_id++;
        }
    }

    primary.close();
}
class Appointment {
public:
    char Appointment_ID[15];
    char Appointment_Date[30];
    char Doctor_ID[15];

    const static int maxRecordSize = 1000;

    void writeAppointment(fstream& file, Appointment& s) {
        char record[maxRecordSize];
        strcpy_s(record, s.Appointment_ID);
        strcat_s(record, "|");
        strcat_s(record, s.Appointment_Date);
        strcat_s(record, "|");
        strcat_s(record, s.Doctor_ID);
        strcat_s(record, "|");

        short length = strlen(record);

        file.write((char*)&length, sizeof(length));
        file.write(record, length);
    }

    void readAppointment(fstream& file, Appointment& s) {
        short length;
        file.read((char*)&length, sizeof(length));
        char* record = new char[length];
        file.read(record, length);

        istrstream strbuff(record);
        strbuff >> s;
        delete[] record;
    }

    friend istream& operator>>(istream& file, Appointment& s) {
        file.getline(s.Appointment_ID, 15, '|');
        file.getline(s.Appointment_Date, 30, '|');
        file.getline(s.Doctor_ID, 15, '|');
        return file;
    }
};
// secondary index on doctor id in appointment
void displaySecondary_DoctorId() {
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in);
    if (!DoctorSec.is_open()) {
        cout << "Secondary Index file not found.\n";
        return;
    }

    char doctorID[15];
    short linkedListPointer;

    while (DoctorSec.read(doctorID, 15)) {
        DoctorSec.read((char*)&linkedListPointer, sizeof(linkedListPointer));
        cout << "Doctor ID: " << doctorID << ", Linked List Pointer: " << linkedListPointer << endl;
    }
    DoctorSec.close();
}
void addToLinkedList_DoctorId(const char* Doctor_ID, const char* Appointment_ID) {
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    if (!DoctorLink.is_open()) {
        cout << "Error opening linked list file." << endl;
        return;
    }
    DoctorLink.seekg(0, ios::end);
    DoctorLink.write(Doctor_ID, 15);
    DoctorLink.write(Appointment_ID, 15);
    short nega = -1;
    DoctorLink.write((char*)&nega, sizeof(nega));
    DoctorLink.close();
}

// totalEntries of secondary index on doctor id in appointment
int totalEntriesOfSecApp = getRecordCount("SecondaryIndexForDoctor.txt", 17);
void insert_secondary_DoctorId(char Doctor_ID[15], char Appointment_ID[15]) {
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in | ios::out);

    char temp[15];
    short linkedListPointer;

    // Binary search for the doctor ID
    bool found = false;
    int insertPosition = totalEntriesOfSecApp;
    for (int i = 0; i < totalEntriesOfSecApp; i++) {
        DoctorSec.seekg(i * 17, ios::beg);
        DoctorSec.read(temp, 15);
        if (strcmp(temp, Doctor_ID) > 0) {
            insertPosition = i;
            found = false;
            break;
        }
        else if (strcmp(temp, Doctor_ID) == 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        // Shift records to the right to make room for the new record
        for (int i = totalEntriesOfSecApp - 1; i >= insertPosition; i--) {
            DoctorSec.seekg(i * 17, ios::beg);
            char existingDoctorID[15];
            short existingPointer;
            DoctorSec.read(existingDoctorID, 15);
            DoctorSec.read((char*)&existingPointer, sizeof(existingPointer));

            DoctorSec.seekp((i + 1) * 17, ios::beg);
            DoctorSec.write(existingDoctorID, 15);
            DoctorSec.write((char*)&existingPointer, sizeof(existingPointer));
        }

        // Insert the new Doctor ID and pointer at the correct position
        DoctorSec.seekp(insertPosition * 17, ios::beg);
        DoctorSec.write(Doctor_ID, 15);
        linkedListPointer = getRecordCount("LLIndexForDoctor.txt", 32);
        DoctorSec.write((char*)&linkedListPointer, sizeof(linkedListPointer));

        // Add the new appointment to the linked list
        addToLinkedList_DoctorId(Doctor_ID, Appointment_ID);
    }
    else
    {
        // Doctor ID exists, update the linked list
        DoctorLink.seekg(0, ios::end);
        int fileSize = DoctorLink.tellg();
        int entrySize = 32;  // Size of each record (doctorID + appointmentID + nextPointer)
        int recordCount = fileSize / entrySize;

        for (int i = recordCount - 1; i >= 0; i--) {
            DoctorLink.seekg(i * entrySize, ios::beg);
            char doctorID[15];
            char appointmentID[15];
            short nextPointer;

            DoctorLink.read(doctorID, 15);
            DoctorLink.read(appointmentID, 15);
            DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

            if (strcmp(doctorID, Doctor_ID) == 0) {
                short newPointer = recordCount; // Update to the next appointment record
                DoctorLink.seekp(i * entrySize + 30, ios::beg);
                DoctorLink.write((char*)&newPointer, sizeof(newPointer));
                break;
            }
        }
        // Add the appointment to the linked list
        addToLinkedList_DoctorId(Doctor_ID, Appointment_ID);
    }
    DoctorSec.close();
    DoctorLink.close();
}


void displayLinkedList_DoctorId() {
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in);
    if (!DoctorLink.is_open()) {
        cout << "Linked List file not found.\n";
        return;
    }

    char doctorID[15];
    char appointmentID[15];
    short nextPointer;

    // Read the content of the linked list file
    while (DoctorLink.read(doctorID, 15)) {
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

        cout << "Doctor ID: " << doctorID
            << ", Appointment ID: " << appointmentID
            << ", Next Pointer: " << nextPointer << endl;
    }
    DoctorLink.close();
}
void write_Number_of_appointment() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Open the file to append data and also read it to determine the current number of records
    fstream file("Appointment.txt", ios::in | ios::out | ios::binary | ios::ate);
    int count_id = 0;

    // Calculate the current number of records if the file is not empty
    if (file.tellg() > 0) {
        file.seekg(0, ios::end);
        count_id = file.tellg() / sizeof(Appointment); // Assuming Appointment size is fixed
    }

    cout << "Enter # of Appointment you want to enter" << endl;
    int count;
    cin >> count;
    cin.ignore();

    Appointment record;

    // Loop to take input for new records
    for (int i = 0; i < count; i++) {
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
        Insert_Appointment_ID_Sorted(record.Appointment_ID, file.tellp());
        insert_secondary_DoctorId(record.Doctor_ID, record.Appointment_ID);

        // Write the new record to the file
        record.writeAppointment(file, record);
    }

    file.close();

    // Open the file again to display its contents
    file.open("Appointment.txt", ios::in | ios::binary);
    Appointment s2;

    cout << "Content of appointment file that inserted:" << "\n";
    for (int i = 0; i < count_id; i++) {
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
short getOffsetFromPrimaryIndex(const char* filename, char id[15]) {
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
}
void searchAppointmentsByDoctorID(const char* Doctor_ID) {
    fstream DoctorSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in);
    fstream DoctorLink("LLIndexForDoctor.txt", ios::binary | ios::in);
    fstream AppointmentFile("Appointment.txt", ios::binary | ios::in);

    if (!DoctorSec.is_open() || !DoctorLink.is_open() || !AppointmentFile.is_open()) {
        cout << "Error: Unable to open one or more files.\n";
        return;
    }

    // search in the secondary index to check if this id exist   and keep linkedListPointer that return first pointer to linked list
    short first = 0, last = getRecordCount("SecondaryIndexForDoctor.txt", 17) - 1;
    char tempDoctorID[15];
    short linkedListPointer = -1;
    bool found = false;

    while (first <= last) {
        short mid = (first + last) / 2;
        DoctorSec.seekg(mid * 17, ios::beg);
        DoctorSec.read(tempDoctorID, 15);

        int cmpResult = strcmp(tempDoctorID, Doctor_ID);
        if (cmpResult == 0) {
            DoctorSec.read((char*)&linkedListPointer, sizeof(linkedListPointer));
            found = true;
            break;
        }
        else if (cmpResult > 0) {
            last = mid - 1;
        }
        else {
            first = mid + 1;
        }
    }

    if (!found) {
        cout << "Doctor ID not found in the secondary index.\n";
        DoctorSec.close();
        DoctorLink.close();
        AppointmentFile.close();
        return;
    }

    // loop in linked list  while pointer !=-1
    cout << "Appointments for Doctor ID: " << Doctor_ID << endl;
    while (linkedListPointer != -1) {
        DoctorLink.seekg(linkedListPointer * 32, ios::beg);

        char doctorID[15];
        char appointmentID[15];
        short nextPointer;

        DoctorLink.read(doctorID, 15);
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

        if (strcmp(doctorID, Doctor_ID) != 0) {
            cout << "Error: Mismatched doctor ID in linked list.\n";
            break;
        }

        // get offset appointment ID from the primary index
        short offset = getOffsetFromPrimaryIndex("appointment_ID_primary_Index.txt", appointmentID);

        if (offset == -1) {
            cout << "Error: Appointment ID " << appointmentID << " not found in primary index.\n";
        }
        else
        {

            AppointmentFile.seekg(offset, ios::beg);
            short length;
            AppointmentFile.read((char*)&length, sizeof(length));

            char* record = new char[length];
            AppointmentFile.read(record, length);

            //Convert the Data to an Appointment Object
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
void testSearch_DI() {
    char doctorID[15];
    cout << "Enter Doctor ID to search: ";

    cin >> setw(15) >> doctorID; // Limit input to 15 characters to prevent buffer overflow

    // Test case
    cout << "Running test for Doctor ID: " << doctorID << endl;
    searchAppointmentsByDoctorID(doctorID);
    cout << "Test complete.\n";
}
// doctor part
class Doctor {
public:
    char Doctor_ID[15];
    char Doctor_Name[30];
    char Address[30];

    const static int maxRecordSize = 1000;

    void writeDoctor(fstream& file, Doctor& d) {
        char record[maxRecordSize];
        strcpy_s(record, d.Doctor_ID);

        (record, "|");
        strcat_s(record, d.Doctor_Name);
        strcat_s(record, "|");
        strcat_s(record, d.Address);
        strcat_s(record, "|");

        short length = strlen(record);

        file.write((char*)&length, sizeof(length));
        file.write(record, length);
    }

    void readDoctor(fstream& file, Doctor& d) {
        short length;
        file.read((char*)&length, sizeof(length));
        char* record = new char[length];
        file.read(record, length);

        istrstream strbuff(record);
        strbuff >> d;
        delete[] record;
    }

    friend istream& operator>>(istream& file, Doctor& d) {
        file.getline(d.Doctor_ID, 15, '|');
        file.getline(d.Doctor_Name, 30, '|');
        file.getline(d.Address, 30, '|');
        return file;
    }

    friend ostream& operator<<(ostream& os, const Doctor& d) {
        os << "Doctor ID: " << d.Doctor_ID << endl;
        os << "Doctor Name: " << d.Doctor_Name << endl;
        os << "Address: " << d.Address << endl;
        return os;
    }
};
void Insert_Doctor_ID_Sorted(char id[], short offset, int& count_id) {
    fstream primary("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

    // Convert the Doctor ID from char array to a comparable integer-like value
    long long New_Doctor_id = 0;
    for (int i = 0; id[i] != '\0'; i++) {
        New_Doctor_id *= 256; // Using base 256 to handle the full character set
        New_Doctor_id += id[i]; // Add each character's ASCII value
    }

    long long existing_id = 0;
    short off = 0;
    bool find_position = false;

    if (count_id == 0) {
        // If this is the first record, directly insert it
        primary.write((char*)&New_Doctor_id, sizeof(New_Doctor_id));
        primary.write((char*)&offset, sizeof(offset));
        count_id++;
    }
    else {
        // Locate the insertion position
        primary.read((char*)&existing_id, sizeof(existing_id));
        while (primary.good()) {
            if (existing_id > New_Doctor_id) {
                find_position = true;
                primary.seekg(-sizeof(existing_id), ios::cur); // Step back to insertion point
                off = primary.tellg();
                break;
            }
            primary.seekg(sizeof(short), ios::cur); // Skip offset
            primary.read((char*)&existing_id, sizeof(existing_id));
        }
        primary.close();
        primary.open("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);

        if (!find_position) {
            // Insert at the end if no suitable position was found
            primary.seekg(count_id * (sizeof(long long) + sizeof(short)), ios::beg);
            primary.write((char*)&New_Doctor_id, sizeof(long long));
            primary.write((char*)&offset, sizeof(short));
            count_id++;
        }
        else {
            // Shift records to make space for the new record
            primary.seekg((count_id - 1) * (sizeof(long long) + sizeof(short)), ios::beg);
            long long endNum;
            short endOf;
            primary.read((char*)&endNum, sizeof(endNum));
            primary.read((char*)&endOf, sizeof(endOf));
            primary.seekg(off);

            while (primary.good()) {
                long long num1;
                short num1_Of;
                long long num2;
                short num2_Of;

                primary.read((char*)&num1, sizeof(num1));
                primary.read((char*)&num1_Of, sizeof(num1_Of));

                primary.read((char*)&num2, sizeof(num2));
                primary.read((char*)&num2_Of, sizeof(num2_Of));

                primary.seekg(-2 * (sizeof(long long) + sizeof(short)), ios::cur);
                primary.write((char*)&num1, sizeof(num1));
                primary.write((char*)&num1_Of, sizeof(num1_Of));
            }

            primary.close();
            primary.open("doctor_ID_primary_Index.txt", ios::out | ios::binary | ios::in);
            primary.seekg(0, ios::end);
            primary.write((char*)&endNum, sizeof(endNum));
            primary.write((char*)&endOf, sizeof(endOf));
            primary.seekg(off);
            primary.write((char*)&New_Doctor_id, sizeof(New_Doctor_id));
            primary.write((char*)&offset, sizeof(off));
            count_id++;
        }
    }

    primary.close();
}
void print_Doctor_ID_Primary_Index() {
    fstream primary("doctor_ID_primary_Index.txt", ios::in | ios::binary);

    if (!primary) {
        std::cout << "Failed to open the primary index file." << std::endl;
        return;
    }

    long long doctor_id = 0;
    short offset = 0;

    std::cout << "Content of Doctor ID Primary Index:" << std::endl;
    int record_count = 1;
    while (primary.read((char*)&doctor_id, sizeof(doctor_id)) && primary.read((char*)&offset, sizeof(offset))) {
        std::cout << "Record " << record_count++ << ":" << std::endl;

        // Convert the doctor ID back to a string
        char doctor_id_str[16]; // Buffer for the Doctor ID
        int idx = 0;
        while (doctor_id > 0) {
            doctor_id_str[idx++] = static_cast<char>(doctor_id % 256); // Extract each byte
            doctor_id /= 256;
        }
        doctor_id_str[idx] = '\0';

        std::cout << "Doctor ID: " << doctor_id_str << std::endl;
        std::cout << "Offset: " << offset << std::endl;
        std::cout << "----------------------------" << std::endl;
    }

    if (primary.eof()) {
        std::cout << "End of index file reached." << std::endl;
    }
    else {
        std::cout << "Error reading from the file." << std::endl;
    }

    primary.close();
}
//-------------------------------- secondary index
// Function to display the secondary index (Doctor_Name with pointer)
void displaySecondary_DoctorName() {
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in);
    if (!DoctorSec.is_open()) {
        cout << "Secondary Index file not found.\n";
        return;
    }

    char doctorName[30]; // Adjust size to match Doctor_Name length
    short linkedListPointer;

    while (DoctorSec.read(doctorName, 30)) {
        DoctorSec.read((char*)&linkedListPointer, sizeof(linkedListPointer));
        cout << "Doctor Name: " << doctorName << ", Linked List Pointer: " << linkedListPointer << endl;
    }
    DoctorSec.close();
}
// Function to add a new record to the linked list for Doctor_Name
void addToLinkedList_DoctorName(const char* Doctor_Name, const char* Appointment_ID) {
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in | ios::out);
    if (!DoctorLink.is_open()) {
        cout << "Error opening linked list file." << endl;
        return;
    }
    DoctorLink.seekg(0, ios::end);
    DoctorLink.write(Doctor_Name, 30); // Store the Doctor_Name
    DoctorLink.write(Appointment_ID, 15); // Store the Appointment_ID
    short nega = -1; // End of linked list marker
    DoctorLink.write((char*)&nega, sizeof(nega)); // Pointer to next (initially -1)
    DoctorLink.close();
}
// Function to insert a new Doctor_Name with pointer to the linked list in the secondary index
void insert_secondary_DoctorName(char Doctor_Name[30], char Appointment_ID[15]) {
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in | ios::out);
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in | ios::out);

    char temp[30];
    short linkedListPointer;
    int totalEntriesOfSecApp = getRecordCount("SecondaryIndexForDoctorName.txt", 32); // 30 + 2 (pointer size)

    // Binary search for the Doctor Name in the secondary index
    bool found = false;
    int insertPosition = totalEntriesOfSecApp;
    for (int i = 0; i < totalEntriesOfSecApp; i++) {
        DoctorSec.seekg(i * 32, ios::beg); // Each entry is 32 bytes (30 for name, 2 for pointer)
        DoctorSec.read(temp, 30);
        if (strcmp(temp, Doctor_Name) > 0) {
            insertPosition = i;
            found = false;
            break;
        }
        else if (strcmp(temp, Doctor_Name) == 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        // Shift records to the right to make room for the new record
        for (int i = totalEntriesOfSecApp - 1; i >= insertPosition; i--) {
            DoctorSec.seekg(i * 32, ios::beg);
            char existingDoctorName[30];
            short existingPointer;
            DoctorSec.read(existingDoctorName, 30);
            DoctorSec.read((char*)&existingPointer, sizeof(existingPointer));

            DoctorSec.seekp((i + 1) * 32, ios::beg);
            DoctorSec.write(existingDoctorName, 30);
            DoctorSec.write((char*)&existingPointer, sizeof(existingPointer));
        }

        // Insert the new Doctor Name and pointer at the correct position
        DoctorSec.seekp(insertPosition * 32, ios::beg);
        DoctorSec.write(Doctor_Name, 30);
        linkedListPointer = getRecordCount("LLIndexForDoctorName.txt", 47); // Get the current position in linked list
        DoctorSec.write((char*)&linkedListPointer, sizeof(linkedListPointer));

        // Add the new appointment to the linked list
        addToLinkedList_DoctorName(Doctor_Name, Appointment_ID);
    }
    else {
        // Doctor Name exists, update the linked list
        DoctorLink.seekg(0, ios::end);
        int fileSize = DoctorLink.tellg();
        int entrySize = 47;  // Size of each record (DoctorName + AppointmentID + NextPointer)
        int recordCount = fileSize / entrySize;

        for (int i = recordCount - 1; i >= 0; i--) {
            DoctorLink.seekg(i * entrySize, ios::beg);
            char doctorName[30];
            char appointmentID[15];
            short nextPointer;

            DoctorLink.read(doctorName, 30);
            DoctorLink.read(appointmentID, 15);
            DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

            if (strcmp(doctorName, Doctor_Name) == 0) {
                short newPointer = recordCount; // Update to the next appointment record
                DoctorLink.seekp(i * entrySize + 45, ios::beg);
                DoctorLink.write((char*)&newPointer, sizeof(newPointer));
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
void displayLinkedList_DoctorName() {
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in);
    if (!DoctorLink.is_open()) {
        cout << "Linked List file not found.\n";
        return;
    }

    char doctorName[30]; // Adjust to match Doctor_Name length
    char appointmentID[15];
    short nextPointer;

    // Read the content of the linked list file
    while (DoctorLink.read(doctorName, 30)) {
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

        cout << "Doctor Name: " << doctorName
            << ", Appointment ID: " << appointmentID
            << ", Next Pointer: " << nextPointer << endl;
    }
    DoctorLink.close();
}
// search
void searchAppointmentsByDoctorName(const char* Doctor_Name) {
    fstream DoctorSec("SecondaryIndexForDoctorName.txt", ios::binary | ios::in);
    fstream DoctorLink("LLIndexForDoctorName.txt", ios::binary | ios::in);
    fstream AppointmentFile("Appointment.txt", ios::binary | ios::in);

    if (!DoctorSec.is_open() || !DoctorLink.is_open() || !AppointmentFile.is_open()) {
        cout << "Error: Unable to open one or more files.\n";
        return;
    }

    // Search in the secondary index to check if this name exists, and keep linkedListPointer that returns the first pointer to the linked list
    short first = 0, last = getRecordCount("SecondaryIndexForDoctorName.txt", 32) - 1; // 32 for DoctorName (30) + pointer (2)
    char tempDoctorName[30]; // Assuming Doctor_Name is of length 30
    short linkedListPointer = -1;
    bool found = false;

    while (first <= last) {
        short mid = (first + last) / 2;
        DoctorSec.seekg(mid * 32, ios::beg); // Each entry is 32 bytes
        DoctorSec.read(tempDoctorName, 30);

        int cmpResult = strcmp(tempDoctorName, Doctor_Name);
        if (cmpResult == 0) {
            DoctorSec.read((char*)&linkedListPointer, sizeof(linkedListPointer));
            found = true;
            break;
        }
        else if (cmpResult > 0) {
            last = mid - 1;
        }
        else {
            first = mid + 1;
        }
    }

    if (!found) {
        cout << "Doctor Name not found in the secondary index.\n";
        DoctorSec.close();
        DoctorLink.close();
        AppointmentFile.close();
        return;
    }

    // Loop through linked list while pointer != -1
    cout << "Appointments for Doctor Name: " << Doctor_Name << endl;
    while (linkedListPointer != -1) {
        DoctorLink.seekg(linkedListPointer * 47, ios::beg); // 47 bytes per record (DoctorName + AppointmentID + NextPointer)

        char doctorName[30];
        char appointmentID[15];
        short nextPointer;

        DoctorLink.read(doctorName, 30);
        DoctorLink.read(appointmentID, 15);
        DoctorLink.read((char*)&nextPointer, sizeof(nextPointer));

        if (strcmp(doctorName, Doctor_Name) != 0) {
            cout << "Error: Mismatched doctor name in linked list.\n";
            break;
        }

        // Get offset for appointment ID from the primary index
        short offset = getOffsetFromPrimaryIndex("appointment_ID_primary_Index.txt", appointmentID);

        if (offset == -1) {
            cout << "Error: Appointment ID " << appointmentID << " not found in primary index.\n";
        }
        else {
            AppointmentFile.seekg(offset, ios::beg);
            short length;
            AppointmentFile.read((char*)&length, sizeof(length));

            char* record = new char[length];
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
void testSearch_DN() {
    char doctorName[30];
    cout << "Enter Doctor Name to search: ";

    cin >> setw(30) >> doctorName; // Limit input to 30 characters to prevent buffer overflow

    // Test case
    cout << "Running test for Doctor Name: " << doctorName << endl;
    searchAppointmentsByDoctorName(doctorName);
    cout << "Test complete.\n";
}
void write_Number_of_Doctor() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Open the file to append data and also read it to determine the current number of records
    fstream file("Doctor.txt", ios::in | ios::out | ios::binary | ios::ate);
    int count_id = 0;

    // Calculate the current number of records if the file is not empty
    if (file.tellg() > 0) {
        file.seekg(0, ios::end);
        count_id = file.tellg() / sizeof(Doctor); // Assuming Doctor size is fixed
    }

    cout << "Enter # of Doctors you want to enter" << endl;
    int count;
    cin >> count;
    cin.ignore();

    Doctor record;

    // Loop to take input for new records
    for (int i = 0; i < count; i++) {
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
        Insert_Doctor_ID_Sorted(record.Doctor_ID, file.tellp(), count_id);
        insert_secondary_DoctorName(record.Doctor_Name, record.Doctor_ID);

        // Write the new record to the file
        record.writeDoctor(file, record);
    }

    file.close();

    // Open the file again to display its contents
    file.open("Doctor.txt", ios::in | ios::binary);
    Doctor s2;

    cout << "Content of doctor file that was inserted:" << "\n";
    for (int i = 0; i < count_id; i++) {
        cout << "Record " << i + 1 << endl;
        s2.readDoctor(file, s2);
        cout << "Doctor ID: " << s2.Doctor_ID << endl;
        cout << "Doctor Name: " << s2.Doctor_Name << endl;
        cout << "Address: " << s2.Address << endl;
        cout << "-------------------------\n";
    }

    file.close();
}


/* Add, Update, Delete from Appointment */

// delete appointment from its primary index
void deleteApointmentPrimary(int ID1) { // delete from primary index
    fstream prim("primary_Index.txt", ios::in | ios::binary | ios::out);
    short first = 0;
    short last = count_id - 1;
    short mid;
    bool found = false;
    int temp;
    while (first <= last && !found) {
        mid = (first + last) / 2;
        prim.seekg(mid * 6, ios::beg);
        prim.read((char*)&temp, sizeof(temp));
        if (temp == ID1)
            found = true;
        else if (temp > ID1)
            last = mid - 1;
        else
            first = mid + 1;
    }
    if (found) {
        prim.seekg((mid + 1) * 6, ios::beg);
        while (prim.good()) { // start to shift
            int tmpnum; short tmpof;
            prim.read((char*)&tmpnum, sizeof(tmpnum));
            prim.read((char*)&tmpof, sizeof(tmpof));

            prim.seekg(-12, ios::cur);
            prim.write((char*)&tmpnum, sizeof(tmpnum));
            prim.write((char*)&tmpof, sizeof(tmpof));
            prim.seekg(6, ios::cur);

        }
        prim.close();
        prim.open("primary_Index.txt", ios::out | ios::in | ios::binary);
        count_id--;
    }
    prim.close();
}

// delete appointment from its secondary index
void deleteAppointmentSecondary(char appointmentId[20]) {
    fstream appSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    short first = 0;
    short last = totalEntriesOfSecApp - 1;
    short mid;
    bool found = false;
    char temp[20];
    while (first <= last && !found) {
        mid = (first + last) / 2;
        appSec.seekg(mid * 22, ios::beg);
        appSec.read((char*)&temp, sizeof(temp));

        if (strcmp(temp, appointmentId) == 0)
            found = true;
        else if (strcmp(temp, appointmentId) == 1)
            last = mid - 1;
        else
            first = mid + 1;
    }
    appSec.close();
    if (!found)
        return;
    appSec.open("SecondaryIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    appSec.seekg(((totalEntriesOfSecApp - 1) * 22), ios::beg);
    appSec.seekg((mid + 22), ios::beg);
    int i = mid / 22;
    while (i < totalEntriesOfSecApp - 1) { // start to shift
        char tempname[20];
        short tempof;
        appSec.read(tempname, 20);
        appSec.read((char*)&tempof, sizeof(tempof));
        appSec.seekg(-44, ios::cur);
        appSec.write(tempname, 20);
        appSec.write((char*)&tempof, sizeof(tempof));
        appSec.seekg(22, ios::cur);
        i++;
    }
    appSec.close();
    totalEntriesOfSecApp--;
    appSec.close();
}

// delete appointment
// add its offset & size in the appropriate place in the avail list
void deleteAppointment(char id[]) {
    short M = -1;
    fstream file;
    file.open("Appointment.txt", ios::in | ios::out | ios::binary);
    short offset = getOffsetFromPrimaryIndex("primary_Index.txt", id), size; // read offset
    file.seekg(offset, ios::beg);
    file.seekg(offset - 2, ios::beg);
    file.read((char*)&size, sizeof(size)); // read size
    if (offset == -1) {
        cout << "Not valid ID " << endl;
    }
    else {
        file.seekg(0, ios::beg);
        short header;
        file.read((char*)&header, sizeof(header)); // read header
        if (header == -1) { // if list is empty
            file.seekg(0, ios::beg);
            file.write((char*)&offset, sizeof(offset));
            file.seekg(offset, ios::beg);
            file.write("*", 1);
            file.write((char*)&M, sizeof(M));
            file.close();
        }
        else { // list is not empty
            file.seekg(0, ios::beg);
            file.read((char*)&header, sizeof(header)); // read header
            short HeaderSize;
            file.seekg(header - 2, ios::beg);
            file.read((char*)&HeaderSize, sizeof(HeaderSize)); // read header size
            if (size >= HeaderSize) { // if deleted size >= header size
                file.seekg(0, ios::beg);
                file.write((char*)&offset, sizeof(offset)); // write offset in the head
                file.seekg(offset, ios::beg);
                file.write("*", 1);
                file.write((char*)&header, sizeof(header)); // write header offset in deleted item
                file.close();
            }
            else { // deleted size  < header size
                // prev new next
                file.seekg(0, ios::beg);
                short offsetPre, sizeNext, tempOffset;
                offsetPre = header;
                file.seekg(header + 1, ios::beg);
                while (1) {
                    file.read((char*)&tempOffset, sizeof(tempOffset));
                    if (tempOffset == -1) { // if reached the end of list
                        file.seekg(offset, ios::beg);
                        file.write("*", 1);
                        file.write((char*)&M, sizeof(M));
                        file.seekg(offsetPre + 1, ios::beg);
                        file.write((char*)&offset, sizeof(offset));
                        file.close();
                        break;
                    }
                    else {
                        file.seekg(tempOffset - 2, ios::beg);
                        file.read((char*)&sizeNext, sizeof(sizeNext));
                        if (size >= sizeNext) { // if  temp size <= deleted size
                            file.seekg(offset, ios::beg);
                            file.write("*", 1);
                            file.write((char*)&tempOffset, sizeof(tempOffset));
                            file.seekg(offsetPre + 1, ios::beg);
                            file.write((char*)&offset, sizeof(offset));
                            file.close();
                            break;
                        }
                        else { // move to the next record
                            offsetPre = tempOffset;
                            file.seekg(offsetPre + 1, ios::beg);
                        }
                    }
                }
                file.close();
            }
        }
        deleteApointmentPrimary((int)id);
        deleteAppointmentSecondary((char*)id);
    }
}

// add new appointment
// reflected in the file and its primary index
// using avail list
void addAppointment(Appointment newAppointment) {
    fstream out("Appointment.txt", ios::out | ios::binary | ios::in);
    short recordSize, appIdSize, docIdSize, dateSize;
    short header;
    out.seekg(0, ios::beg);
    out.read((char*)&header, sizeof(header));
    appIdSize = strlen(newAppointment.Appointment_ID);
    docIdSize = strlen(newAppointment.Doctor_ID);
    dateSize = strlen(newAppointment.Appointment_Date);
    recordSize = appIdSize + docIdSize + dateSize + 10; /// calculate record length
    if (header == -1) { /// if avail list is empty we will insert at the end
        out.seekp(0, ios::end);
        out.write((char*)&recordSize, sizeof(recordSize));
        short end1 = out.tellp();
        out.write((char*)&appIdSize, sizeof(appIdSize));
        out.write(newAppointment.Appointment_ID, appIdSize);
        out.write((char*)&docIdSize, sizeof(docIdSize));
        out.write(newAppointment.Doctor_ID, docIdSize);
        out.write((char*)&dateSize, sizeof(dateSize));
        out.write(newAppointment.Appointment_Date, dateSize);
        Insert_Appointment_ID_Sorted(newAppointment.Appointment_ID, end1); // add in the primary index
        insert_secondary_DoctorId(newAppointment.Doctor_ID, newAppointment.Appointment_ID); // add in the primary index
        out.close();
    }
    else { // if the avail list is not empty
        short nextof, deletedsize;
        out.seekg(header + 1, ios::beg);
        out.read((char*)&nextof, sizeof(nextof)); // read next offset after the header
        out.seekg(header - 2, ios::beg);
        out.read((char*)&deletedsize, sizeof(deletedsize)); // read header size
        short differSize = deletedsize - recordSize; // the difference between my size and header size
        if (differSize < 0) { // if my size > headers we will insert at the end
            out.seekp(0, ios::end);
            out.write((char*)&recordSize, sizeof(recordSize));
            Insert_Appointment_ID_Sorted(newAppointment.Appointment_ID, out.tellp()); // add in the primary index
            insert_secondary_DoctorId(newAppointment.Doctor_ID, newAppointment.Appointment_ID); // add in the secondary index
            out.write((char*)&appIdSize, sizeof(appIdSize));
            out.write(newAppointment.Appointment_ID, appIdSize);
            out.write((char*)&docIdSize, sizeof(docIdSize));
            out.write(newAppointment.Doctor_ID, docIdSize);
            out.write((char*)&dateSize, sizeof(dateSize));
            out.write(newAppointment.Appointment_Date, dateSize);
            out.close();
        }
        else {
            if (differSize - 2 <= 5) { // if the difference  <= 5
                out.seekp(0, ios::beg);
                out.write((char*)&nextof, sizeof(nextof));
                out.seekp(header - 2, ios::beg);
                Insert_Appointment_ID_Sorted(newAppointment.Appointment_ID, header); // add in the primary index
                insert_secondary_DoctorId(newAppointment.Doctor_ID, newAppointment.Appointment_ID); // add in the secondary index
                recordSize += differSize;
                out.write((char*)&recordSize, sizeof(recordSize));
                out.write((char*)&appIdSize, sizeof(appIdSize));
                out.write(newAppointment.Appointment_ID, appIdSize);
                out.write((char*)&docIdSize, sizeof(docIdSize));
                out.write(newAppointment.Doctor_ID, docIdSize);
                out.write((char*)&dateSize, sizeof(dateSize));
                out.write(newAppointment.Appointment_Date, dateSize);
                out.close();
            }
            else { // if the difference > 5
                out.seekp(0, ios::beg);
                out.write((char*)&nextof, sizeof(nextof)); // write the new header
                out.seekp(header - 2, ios::beg); // write the new element
                Insert_Appointment_ID_Sorted(newAppointment.Appointment_ID, header); // add in the primary index
                insert_secondary_DoctorId(newAppointment.Doctor_ID, newAppointment.Appointment_ID); // add in the secondary index
                out.write((char*)&recordSize, sizeof(recordSize));
                out.write((char*)&appIdSize, sizeof(appIdSize));
                out.write(newAppointment.Appointment_ID, appIdSize);
                out.write((char*)&docIdSize, sizeof(docIdSize));
                out.write(newAppointment.Doctor_ID, docIdSize);
                out.write((char*)&dateSize, sizeof(dateSize));
                out.write(newAppointment.Appointment_Date, dateSize);
                out.seekg(-(appIdSize + docIdSize + dateSize + 10), ios::cur); // seek to the beg of the record
                out.seekg(recordSize, ios::cur); // seek after the new record size
                short ID_Fake = 1; // make a fake record with the rest of the original record
                differSize -= 2;
                out.write((char*)&differSize, sizeof(differSize));
                out.write((char*)&ID_Fake, sizeof(ID_Fake));
                out.write("#", 1);
                out.write((char*)&ID_Fake, sizeof(ID_Fake));
                out.write("1", 1);
                out.close();
                deleteAppointment((char*)("1")); // then delete it to push in the avail list
            }
        }
    }
}

// update an appointment
void updateAppointment(Appointment updatedAppointment) {
    fstream file("Appointment.txt", ios::in | ios::out | ios::binary);
    short recordSize, appIdSize, docIdSize, dateSize;
    short header;

    // Read the header
    file.seekg(0, ios::beg);
    file.read((char*)&header, sizeof(header));

    // Calculate the size of the updated appointment
    appIdSize = strlen(updatedAppointment.Appointment_ID);
    docIdSize = strlen(updatedAppointment.Doctor_ID);
    dateSize = strlen(updatedAppointment.Appointment_Date);
    recordSize = appIdSize + docIdSize + dateSize + 10; // Record size calculation

    // Get the offset of the appointment from the primary index
    short offset = getOffsetFromPrimaryIndex("primary_Index.txt", updatedAppointment.Appointment_ID);

    if (offset == -1) {
        cout << "Appointment ID not found!" << endl;
        return;
    }

    // Read the current appointment details
    file.seekg(offset, ios::beg);
    short currentSize;
    char oldDoctorID[20];
    char oldDate[20];
    char currentAppointmentID[10];

    file.read((char*)&currentSize, sizeof(currentSize));
    file.read((char*)&appIdSize, sizeof(appIdSize));
    file.read(currentAppointmentID, appIdSize);
    file.read((char*)&docIdSize, sizeof(docIdSize));
    file.read(oldDoctorID, docIdSize);
    file.read((char*)&dateSize, sizeof(dateSize));
    file.read(oldDate, dateSize);

    currentAppointmentID[appIdSize] = '\0';
    oldDoctorID[docIdSize] = '\0';
    oldDate[dateSize] = '\0';

    // Check if the appointment size has changed
    if (currentSize != recordSize) {
        deleteAppointment(updatedAppointment.Appointment_ID); // Delete the old appointment
        addAppointment(updatedAppointment); // Add the updated appointment
    }
    else {
        // Update in place
        file.seekp(offset + sizeof(currentSize), ios::beg);
        file.write((char*)&appIdSize, sizeof(appIdSize));
        file.write(updatedAppointment.Appointment_ID, appIdSize);
        file.write((char*)&docIdSize, sizeof(docIdSize));
        file.write(updatedAppointment.Doctor_ID, docIdSize);
        file.write((char*)&dateSize, sizeof(dateSize));
        file.write(updatedAppointment.Appointment_Date, dateSize);
    }

    // Update indices for the new data
    updateAppointmentInPrimaryIndex(updatedAppointment.Appointment_ID, offset);
    updateAppointmentInSecondaryIndex(updatedAppointment.Doctor_ID, oldDoctorID, updatedAppointment.Doctor_ID);

    file.close();
}

// function to update the primary index for an appointment
void updateAppointmentInPrimaryIndex(const char* appointmentId, short offset) {
    fstream prim("primary_Index.txt", ios::in | ios::out | ios::binary);
    short first = 0;
    short last = count_id - 1;
    short mid;
    bool found = false;
    int temp;

    // Binary search for the appointment ID in the primary index
    while (first <= last && !found) {
        mid = (first + last) / 2;
        prim.seekg(mid * 6, ios::beg);
        prim.read((char*)&temp, sizeof(temp));

        if (temp == atoi(appointmentId))
            found = true;
        else if (temp > atoi(appointmentId))
            last = mid - 1;
        else
            first = mid + 1;
    }

    // Update the appointment if found
    if (found) {
        prim.seekp(mid * 6, ios::beg);
        prim.write((char*)&offset, sizeof(offset));
    }

    prim.close();
}

// function to update the secondary index (Doctor ID) for an appointment
void updateAppointmentInSecondaryIndex(char appointmentID[10], char oldDoctorID[10], char newDoctorID[10]) {
    fstream AppointmentSec("SecondaryIndexForDoctor.txt", ios::binary | ios::in | ios::out);
    short first = 0;
    short last = totalEntriesOfSecApp - 1;
    short mid;
    bool found = false;
    char tempAppointmentID[10];

    // Binary search to locate the appointment ID in the secondary index
    while (first <= last && !found) {
        mid = (first + last) / 2;
        AppointmentSec.seekg(mid * 22, ios::beg);
        AppointmentSec.read(tempAppointmentID, sizeof(tempAppointmentID));

        if (strcmp(tempAppointmentID, appointmentID) == 0)
            found = true;
        else if (strcmp(tempAppointmentID, appointmentID) > 0)
            last = mid - 1;
        else
            first = mid + 1;
    }

    if (!found) {
        AppointmentSec.close();
        return; // Appointment ID not found
    }

    // Retrieve the offset for the linked list
    short offset;
    AppointmentSec.seekg((mid * 22) + 10, ios::beg);
    AppointmentSec.read((char*)&offset, sizeof(offset));
    AppointmentSec.close();

    // Open the linked list file
    fstream AppointmentLink("LLIndexForAppointment.txt", ios::binary | ios::in | ios::out);
    AppointmentLink.seekg((offset * 32) + 10, ios::beg); // Move to the start of the linked list for this appointment ID

    char tempDoctorID[10];
    short nextOffset;

    // Traverse the linked list to find and update the doctor IDs
    while (true) {
        AppointmentLink.read(tempDoctorID, sizeof(tempDoctorID));
        if (strcmp(tempDoctorID, oldDoctorID) == 0) {
            // Remove the old doctor ID by marking the record
            AppointmentLink.seekg(-32, ios::cur);
            AppointmentLink.write("*", 1);

            // Add the new doctor ID
            AppointmentLink.seekp(0, ios::end); // Move to the end of the file to append
            AppointmentLink.write(newDoctorID, sizeof(newDoctorID));
            AppointmentLink.write((char*)&offset, sizeof(offset)); // Link it to the same appointment offset
            break;
        }

        // Read the next offset
        AppointmentLink.read((char*)&nextOffset, sizeof(nextOffset));
        if (nextOffset == -1) { // End of the linked list
            break;
        }

        AppointmentLink.seekg((nextOffset * 32) + 10, ios::beg); // Move to the next record
    }

    AppointmentLink.close();
}
//------------------------------------------
void printAppointmentInfo() {
    char appointmentID[15];
    cout << "Enter Appointment ID to print the info: ";
    cin >> setw(15) >> appointmentID;

    // Open the appointment file to search for the appointment by ID
    fstream appointmentFile("AppointmentFile.txt", ios::in | ios::binary);
    Appointment appointment; // Assuming Appointment is a class with Appointment_ID, Appointment_Date, etc.

    bool found = false;
    while (appointmentFile.read((char*)&appointment, sizeof(Appointment))) {
        if (strcmp(appointment.Appointment_ID, appointmentID) == 0) {
            found = true;
            // Print appointment details
            cout << "\nAppointment ID: " << appointment.Appointment_ID << endl;
            //cout << "Doctor Name: " << appointment.Doctor_Name << endl;
            cout << "Appointment Date: " << appointment.Appointment_Date << endl;
            break;
        }
    }

    if (!found) {
        cout << "Error: Appointment with ID " << appointmentID << " not found.\n";
    }

    appointmentFile.close();
}

void printDoctorInfo(  ) {
    int doctorID;
    cout << "Enter Doctor ID to print the info: ";
    cin >> doctorID;

    // Open the file where the doctor info is stored
    fstream file("Doctor.txt", ios::in | ios::binary);
    Doctor doctor; // Assuming Doctor is a class with attributes like ID, Name, Address

    bool found = false;
    while (file.read((char*)&doctor, sizeof(Doctor))) {
        if (*doctor.Doctor_ID == doctorID) {
            found = true;
            // Print doctor details
            cout << "\nDoctor ID: " << doctor.Doctor_ID << endl;
            cout << "Doctor Name: " << doctor.Doctor_Name << endl;
            cout << "Address: " << doctor.Address << endl;
            break;
        }
    }

    if (!found) {
        cout << "Error: Doctor with ID " << doctorID << " not found.\n";
    }

    file.close();
}



//------------------------------------------------
void showMainMenu() {
    int choice;
    bool exit = false;

    while (!exit) {
        cout << "\n************************** Welcome to the System **************************"<< endl;
        cout << "1. Add New Doctor" << endl;
        cout << "2. Add New Appointment" << endl;
        cout << "3. Update Doctor Name (Doctor ID)" << endl;
        cout << "4. Update Appointment Date (Appointment ID)" << endl;
        cout << "5. Delete Appointment (Appointment ID)" << endl;
        cout << "6. Delete Doctor (Doctor ID)" << endl;
        cout << "7. Print Doctor Info (Doctor ID)" << endl;
        cout << "8. Print Appointment Info (Appointment ID)" << endl;
        cout << "9. Write Query" << endl;
        cout << "10. Exit" << endl;

        cout << "Enter your choice (1-10): ";
        cin >> choice;

        switch (choice) {
            case 1:
               write_Number_of_Doctor();
                break;
            case 2:
                write_Number_of_appointment() ;
                break;
            case 3:
               // updateDoctorName();
                break;
            case 4:
              //updateAppointment( )
                break;
            case 5:
                //deleteAppointment();
                break;
            case 6:
                //deleteDoctor();
                break;
            case 7:
                // printDoctorInfo();
                break;
            case 8:
               // printAppointmentInfo();
                break;
            case 9:
               // writeQuery();
                break;
            case 10:
                exit = true;
                cout << "Exiting the system. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again!" << endl;
        }
    }
}


int main() {

     showMainMenu();
    // Define variables
    Appointment newAppointment, updatedAppointment;

    // Initialize a new appointment
    strcpy_s(newAppointment.Appointment_ID, "1234");
    strcpy_s(newAppointment.Doctor_ID, "D5678");
    strcpy_s(newAppointment.Appointment_Date, "2024-12-07");

    cout << "Adding a new appointment..." << endl;
    addAppointment(newAppointment); // Call the add function
    cout << "Appointment added successfully." << endl;

    // Update the appointment details
    strcpy_s(updatedAppointment.Appointment_ID, "1234");
    strcpy_s(updatedAppointment.Doctor_ID, "D91011");
    strcpy_s(updatedAppointment.Appointment_Date, "2024-12-08");

    cout << "Updating the appointment..." << endl;
    updateAppointment(updatedAppointment); // Call the update function
    cout << "Appointment updated successfully." << endl;

    // Delete the appointment
    cout << "Deleting the appointment..." << endl;
    deleteAppointment((char*)"1234"); // Call the delete function
    cout << "Appointment deleted successfully." << endl;

    return 0;
}
