#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
using namespace std;

// login credentials
const string ADMIN_USERNAME = "admin";
const string ADMIN_PASSWORD = "admin123";
const string MANAGER_USERNAME = "manager";
const string MANAGER_PASSWORD = "manager123";

const int MAX_LOGIN_ATTEMPTS = 5;

// return values for login
const int ROLE_NONE = 0;
const int ROLE_ADMIN = 1;
const int ROLE_MANAGER = 2;

// choices for admin menu
const int ADMIN_BOOK_SEATS = 1;
const int ADMIN_VIEW_PRICE_LIST = 2;
const int ADMIN_PAYMENT = 3;
const int ADMIN_PAYMENT_DETAILS = 4;
const int ADMIN_CANCEL_SEATS = 5;
const int ADMIN_LOGOUT = 6;

// size of the auditorium
const int NUM_ROWS = 15;
const int NUM_SEATS = 20;

const char SEAT_AVAILABLE = 'A';
const char SEAT_RESERVED = 'R';
const char SEAT_TAKEN = 'X';

// price list and payment files
const string PRICE_FILE = "pricelist.txt";
const string PAYMENT_FILE = "payments.txt";
const string PROMO_FILE = "promocodes.txt";
const string SEATS_FILE = "seats.txt";

struct PriceCategory {
    int startRow;
    int endRow;
    string category;
    double price;
};

struct PromoCode {
    string code;
    double discountPercent;
};

// function prototypes
// login / menu / seats
int loginMenu();
void adminMenu(char seats[NUM_ROWS][NUM_SEATS],
               const vector<PriceCategory> &priceList,
               const vector<PromoCode> &promoList);
void bookSeats(char seats[NUM_ROWS][NUM_SEATS],
               const vector<PriceCategory> &priceList,
               const vector<PromoCode> &promoList);
void cancelSeats(char seats[NUM_ROWS][NUM_SEATS]);
void initializeSeats(char seats[NUM_ROWS][NUM_SEATS]);
void displaySeatMap(char seats[NUM_ROWS][NUM_SEATS]);
bool isValidRowSeat(int row, int seat);
void loadSeats(char seats[NUM_ROWS][NUM_SEATS]);
void saveSeats(char seats[NUM_ROWS][NUM_SEATS]);

// price list 
void loadPriceList(vector<PriceCategory> &priceList);
void createDefaultPriceFile();
void displayPriceList(const vector<PriceCategory> &priceList);
double getPriceForRow(const vector<PriceCategory> &priceList, int row);
double calculatePrice(const vector<PriceCategory> &priceList, int row, int seatCount);

// promo codes 
void loadPromoList(vector<PromoCode> &promoList);
void createDefaultPromoFile();
void displayPromoList(const vector<PromoCode> &promoList);
double getDiscountPercent(const vector<PromoCode> &promoList, const string &code);

// payment 
void makePayment(char seats[NUM_ROWS][NUM_SEATS],
                  const vector<PriceCategory> &priceList, const vector<PromoCode> &promoList);
void processPayment(char seats[NUM_ROWS][NUM_SEATS],
                    const vector<PromoCode> &promoList,
                    string bookingID,
                    string customerName,
                    int bookedRow[],
                    int bookedSeat[],
                    int numBooked,
                    double subtotal);
void viewPaymentDetails();
void viewAllPayments();

// input validation 
int getValidInt(string prompt, int minVal, int maxVal);
double getValidDouble(string prompt, double minVal);

// manager menu 
void manager(char seats[NUM_ROWS][NUM_SEATS]);
void totalRevenue();
void seatsSold(char seats[NUM_ROWS][NUM_SEATS]);
void seatsAvailable(char seats[NUM_ROWS][NUM_SEATS]);
void overallReport(char seats[NUM_ROWS][NUM_SEATS]);

// main
int main() {
    cout << "\n   SCCA THEATRE TICKET SALES SYSTEM\n\n";

    char seats[NUM_ROWS][NUM_SEATS];
    loadSeats(seats);

    vector<PriceCategory> priceList;
    loadPriceList(priceList);

    vector<PromoCode> promoList;
    loadPromoList(promoList);

    int role = loginMenu();

    if (role == ROLE_ADMIN) {
        cout << "\nLogin successful! Welcome, Administrator.\n";
        adminMenu(seats, priceList, promoList);
    }
    else if (role == ROLE_MANAGER) {
        cout << "\nLogin successful! Welcome, Manager.\n";
        manager(seats);
    }
    else {
        cout << "\nToo many failed attempts. Exiting program.\n";
    }

    return 0;
}

// login / admin menu

int loginMenu() {
    string username, password;

    for (int attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; attempt++) {
        cout << "Attempt " << attempt << " of " << MAX_LOGIN_ATTEMPTS << "\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD) {
            return ROLE_ADMIN;
        }
        else if (username == MANAGER_USERNAME && password == MANAGER_PASSWORD) {
            return ROLE_MANAGER;
        }
        else {
            cout << "Invalid username or password. Please try again.\n\n";
        }
    }

    return ROLE_NONE; // failed after 5 attempts
}

void adminMenu(char seats[NUM_ROWS][NUM_SEATS],
               const vector<PriceCategory> &priceList,
               const vector<PromoCode> &promoList) {
    int choice;

    do {
        cout << "\n----- ADMINISTRATOR MENU -----\n";
        cout << ADMIN_BOOK_SEATS << ". Book Seats\n";
        cout << ADMIN_VIEW_PRICE_LIST << ". View Price List\n";
        cout << ADMIN_PAYMENT << ". Payment\n";
        cout << ADMIN_PAYMENT_DETAILS << ". Payment Details\n";
        cout << ADMIN_CANCEL_SEATS << ". Release/Cancel Reserved Seats\n";
        cout << ADMIN_LOGOUT << ". Logout\n";
        choice = getValidInt("Enter your choice: ", 1, 6);

        switch (choice) {
            case ADMIN_BOOK_SEATS:
                bookSeats(seats, priceList, promoList);
                break;
            case ADMIN_VIEW_PRICE_LIST: {
                displayPriceList(priceList);
                int backChoice;
                do {
                    cout << "\nEnter 0 to return to menu: ";
                    cin >> backChoice;
                } while (backChoice != 0);
                break;
            }
            case ADMIN_PAYMENT:
                makePayment(seats, priceList, promoList);
                break;
            case ADMIN_PAYMENT_DETAILS:
                viewPaymentDetails();
                break;
            case ADMIN_CANCEL_SEATS:
                cancelSeats(seats);
                break;
            case ADMIN_LOGOUT:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != ADMIN_LOGOUT);
}

// seat booking
void bookSeats(char seats[NUM_ROWS][NUM_SEATS],
               const vector<PriceCategory> &priceList,
               const vector<PromoCode> &promoList) {
    displaySeatMap(seats);

    const int TOTAL_SEATS = NUM_ROWS * NUM_SEATS;
    int numSeatsToBook;

    numSeatsToBook = getValidInt("How many seats would you like to book? (0 to go back): ",
    0,
    TOTAL_SEATS
    );

    if (numSeatsToBook == 0) {
        cout << "Booking cancelled. Returning to menu.\n";
        return;
    }

    int bookedRow[TOTAL_SEATS];
    int bookedSeat[TOTAL_SEATS];
    int numBooked = 0;

    for (int i = 0; i < numSeatsToBook; i++) {
        int rowInput, seatInput;
        bool seatConfirmed = false;
        bool cancelled = false;

        while (!seatConfirmed && !cancelled) {
            cout << "\nSeat " << (i + 1) << " of " << numSeatsToBook << "\n";
            rowInput = getValidInt(
             "Enter row number (1-" + to_string(NUM_ROWS) + ", 0 to cancel booking and return to menu): ",
             0,
             NUM_ROWS
            );

            if (rowInput == 0) {
                cancelled = true;
                break;
            }

            seatInput = getValidInt(
                "Enter seat number (1-" + to_string(NUM_SEATS) + "): ",
                1,
                NUM_SEATS
            );

            if (!isValidRowSeat(rowInput, seatInput)) {
                cout << "Invalid row or seat number. Please try again.\n";
            }
            else if (seats[rowInput - 1][seatInput - 1] == SEAT_RESERVED) {
                cout << "That seat is already reserved. Please choose another.\n";
            }
            else if (seats[rowInput - 1][seatInput - 1] == SEAT_TAKEN) {
                cout << "That seat has already been sold. Please choose another.\n";
            }
            else {
                seatConfirmed = true;
            }
        }

        if (cancelled) {
            cout << "\nBooking cancelled. ";
            if (numBooked > 0) {
                cout << numBooked << " seat(s) booked so far will remain reserved.\n";
            } else {
                cout << "No seats were booked.\n";
            }
            break;
        }

        seats[rowInput - 1][seatInput - 1] = SEAT_RESERVED;
        bookedRow[numBooked] = rowInput;
        bookedSeat[numBooked] = seatInput;
        numBooked++;
    }

    if (numBooked > 0) {
        cout << "\nSeat(s) booked:\n";
        for (int i = 0; i < numBooked; i++) {
            cout << "  Row " << bookedRow[i] << ", Seat " << bookedSeat[i] << "\n";
        }

        cout << "\nUpdated seat map:\n";
        displaySeatMap(seats);


        double subtotal = 0.0;
        for (int i = 0; i < numBooked; i++) {
            subtotal += getPriceForRow(priceList, bookedRow[i]);
        }

        char proceed;
        cout << "\nProceed to payment now for these " << numBooked << " seat(s)? (y/n): ";
        cin >> proceed;

        if (proceed == 'y' || proceed == 'Y') {
            string bookingID, customerName;
            cout << "Enter Booking ID: ";
            cin.ignore();
            getline(cin, bookingID);
            cout << "Enter Customer Name: ";
            getline(cin, customerName);

            processPayment(seats,
               promoList,
               bookingID,
               customerName,
               bookedRow,
               bookedSeat,
               numBooked,
               subtotal);
        } else {
            char release;
            cout << "\nRelease these seat(s) back to available instead of\n"
                    "leaving them reserved? (y/n): ";
            cin >> release;

            if (release == 'y' || release == 'Y') {
                for (int i = 0; i < numBooked; i++) {
                    seats[bookedRow[i] - 1][bookedSeat[i] - 1] = SEAT_AVAILABLE;
                }
                cout << "Seat(s) released.\n";
                cout << "\nUpdated seat map:\n";
                displaySeatMap(seats);
            } else {
                cout << "\nOkay, these seat(s) remain reserved. Go to the Payment\n"
                        "option in the menu later to complete payment for them, or\n"
                        "use \"Release/Cancel Reserved Seats\" to free them up.\n";
            }
        }
    }
    saveSeats(seats); // persist booking/reservation changes to disk
}

// cancel seats
void cancelSeats(char seats[NUM_ROWS][NUM_SEATS]) {
    displaySeatMap(seats);
    cout << "Release which seat(s)? Enter row 0 when done.\n";
    while (true) {
        int rowInput, seatInput;
        rowInput = getValidInt(
         "Enter row number (1-" + to_string(NUM_ROWS) + ", 0 to finish): ",
         0,
         NUM_ROWS
        );

        if (rowInput == 0) {
            break;
        }

        seatInput = getValidInt(
            "Enter seat number (1-" + to_string(NUM_SEATS) + "): ",
            1,
            NUM_SEATS
        );

        if (!isValidRowSeat(rowInput, seatInput)) {
            cout << "Invalid row or seat number. Please try again.\n";
            continue;
        }

        if (seats[rowInput - 1][seatInput - 1] == SEAT_AVAILABLE) {
            cout << "Row " << rowInput << ", Seat " << seatInput
             << " is already available - nothing to release.\n";
        }
        else if (seats[rowInput - 1][seatInput - 1] == SEAT_RESERVED) {
            seats[rowInput - 1][seatInput - 1] = SEAT_AVAILABLE;
            cout << "Reserved seat released successfully.\n";
        }
        else if (seats[rowInput - 1][seatInput - 1] == SEAT_TAKEN) {
            cout << "This seat has already been paid for and cannot be released "
             << "using the reserved-seat cancellation option.\n";
        }
    }
    cout << "\nUpdated seat map:\n";
    displaySeatMap(seats);
    saveSeats(seats); // persist release/cancel changes to disk
}

void initializeSeats(char seats[NUM_ROWS][NUM_SEATS]) {
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int seat = 0; seat < NUM_SEATS; seat++) {
            seats[row][seat] = SEAT_AVAILABLE;
        }
    }
}

void displaySeatMap(char seats[NUM_ROWS][NUM_SEATS]) {
    cout << "\n";
    cout << "                              SEATS\n";

    cout << "        ";
    for (int seat = 1; seat <= NUM_SEATS; seat++) {
        cout.width(3);
        cout << seat;
    }
    cout << "\n";

    cout << "         ";
    for (int seat = 1; seat <= NUM_SEATS; seat++) {
        cout << "---";
    }
    cout << "\n";

    for (int row = 0; row < NUM_ROWS; row++) {
        cout << "ROW ";
        cout.width(2);
        cout << (row + 1) << "  ";

        for (int seat = 0; seat < NUM_SEATS; seat++) {
            cout.width(3);
            if (seats[row][seat] == SEAT_AVAILABLE) {
                cout << (seat + 1);
            }
            else if (seats[row][seat] == SEAT_RESERVED) {
                cout << "RR";
            }
            else if (seats[row][seat] == SEAT_TAKEN) {
                cout << "XX";
            }
        }
        cout << "\n";
    }
    cout << "\n";
}

bool isValidRowSeat(int row, int seat) {
    return (row >= 1 && row <= NUM_ROWS && seat >= 1 && seat <= NUM_SEATS);
}

// Reads the saved seat map from SEATS_FILE / create an all-available seat map if the file doesn't exist
void loadSeats(char seats[NUM_ROWS][NUM_SEATS]) {
    ifstream inFile(SEATS_FILE);
    if (!inFile) {
        initializeSeats(seats);
        return;
    }

    string line;
    int row = 0;
    while (row < NUM_ROWS && getline(inFile, line)) {
        for (int seat = 0; seat < NUM_SEATS; seat++) {
            if (seat < (int)line.size() &&
                (line[seat] == SEAT_AVAILABLE ||
                line[seat] == SEAT_RESERVED ||
                line[seat] == SEAT_TAKEN)) {
                seats[row][seat] = line[seat];
            } else {
                seats[row][seat] = SEAT_AVAILABLE; // malformed/missing data, default safe
            }
        }
        row++;
    }
    inFile.close();

    // File was shorter/malformed - fill any remaining rows as available
    for (; row < NUM_ROWS; row++) {
        for (int seat = 0; seat < NUM_SEATS; seat++) {
            seats[row][seat] = SEAT_AVAILABLE;
        }
    }
}

// Writes the current seat map to SEATS_FILE so it persists across runs
void saveSeats(char seats[NUM_ROWS][NUM_SEATS]) {
    ofstream outFile(SEATS_FILE);
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int seat = 0; seat < NUM_SEATS; seat++) {
            outFile << seats[row][seat];
        }
        outFile << "\n";
    }
    outFile.close();
}

// price list
void loadPriceList(vector<PriceCategory> &priceList) {
    ifstream inFile(PRICE_FILE);
    if (!inFile) {
        createDefaultPriceFile();
        inFile.open(PRICE_FILE);
    }

    priceList.clear();
    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        PriceCategory pc;

        getline(ss, token, ','); pc.startRow = stoi(token);
        getline(ss, token, ','); pc.endRow   = stoi(token);
        getline(ss, token, ','); pc.category = token;
        getline(ss, token, ','); pc.price    = stod(token);

        priceList.push_back(pc);
    }
    inFile.close();
}

// Rows 1-5   -> VIP      RM 50.00
// Rows 6-10  -> Standard RM 35.00
// Rows 11-15 -> Economy  RM 20.00
void createDefaultPriceFile() {
    ofstream outFile(PRICE_FILE);
    outFile << "1,5,VIP,50.00\n";
    outFile << "6,10,Standard,35.00\n";
    outFile << "11,15,Economy,20.00\n";
    outFile.close();
}

void displayPriceList(const vector<PriceCategory> &priceList) {
    cout << "\n--------- PRICE LIST ---------\n";
    cout << left << setw(10) << "Rows" << setw(12) << "Category" << "Price (RM)\n";
    cout << "-------------------------------\n";
    for (int i = 0; i < (int)priceList.size(); i++) {
        string rowRange = to_string(priceList[i].startRow) + "-" + to_string(priceList[i].endRow);
        cout << left << setw(10) << rowRange
             << setw(12) << priceList[i].category
             << fixed << setprecision(2) << priceList[i].price << "\n";
    }
    cout << "-------------------------------\n";
}

double getPriceForRow(const vector<PriceCategory> &priceList, int row) {
    for (int i = 0; i < (int)priceList.size(); i++) {
        if (row >= priceList[i].startRow && row <= priceList[i].endRow) {
            return priceList[i].price;
        }
    }
    return 0.0; // invalid row
}

double calculatePrice(const vector<PriceCategory> &priceList, int row, int seatCount) {
    double pricePerSeat = getPriceForRow(priceList, row);
    return pricePerSeat * seatCount;
}

// promo codes
void loadPromoList(vector<PromoCode> &promoList) {
    ifstream inFile(PROMO_FILE);
    if (!inFile) {
        createDefaultPromoFile();
        inFile.open(PROMO_FILE);
    }

    promoList.clear();
    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        PromoCode pc;

        getline(ss, token, ','); pc.code = token;
        getline(ss, token, ','); pc.discountPercent = stod(token);

        promoList.push_back(pc);
    }
    inFile.close();
}

// STUDENT10 -> 10% off, GROUP15 -> 15% off, VIP20 -> 20% off
void createDefaultPromoFile() {
    ofstream outFile(PROMO_FILE);
    outFile << "STUDENT10,10\n";
    outFile << "GROUP15,15\n";
    outFile << "VIP20,20\n";
    outFile.close();
}

void displayPromoList(const vector<PromoCode> &promoList) {
    cout << "\n--------- PROMO CODES ---------\n";
    cout << left << setw(15) << "Code" << "Discount\n";
    cout << "--------------------------------\n";
    for (int i = 0; i < (int)promoList.size(); i++) {
        cout << left << setw(15) << promoList[i].code
             << promoList[i].discountPercent << "%\n";
    }
    cout << "--------------------------------\n";
}

double getDiscountPercent(const vector<PromoCode> &promoList, const string &code) {
    for (int i = 0; i < (int)promoList.size(); i++) {
        if (promoList[i].code == code) {
            return promoList[i].discountPercent;
        }
    }
    return 0.0; // invalid / no code entered
}

// payment
void makePayment(char seats[NUM_ROWS][NUM_SEATS],
                  const vector<PriceCategory> &priceList, const vector<PromoCode> &promoList) {
    cout << "\n--------- BOOK SEATS & PAYMENT ---------\n";

    displaySeatMap(seats);

    const int TOTAL_SEATS = NUM_ROWS * NUM_SEATS;
    int numSeatsToBook = getValidInt("How many seats would you like to book? (0 to return to menu, 1-" +
                                      to_string(TOTAL_SEATS) + "): ", 0, TOTAL_SEATS);

    if (numSeatsToBook == 0) {
        cout << "Returning to menu.\n";
        return;
    }

    string bookingID, customerName;
    cout << "Enter Booking ID: ";
    cin.ignore();
    getline(cin, bookingID);
    cout << "Enter Customer Name: ";
    getline(cin, customerName);

    int bookedRow[TOTAL_SEATS];
    int bookedSeat[TOTAL_SEATS];
    int numBooked = 0;
    bool cancelled = false;

    for (int i = 0; i < numSeatsToBook && !cancelled; i++) {
        int rowInput, seatInput;
        bool seatConfirmed = false;

        while (!seatConfirmed) {
            cout << "\nSeat " << (i + 1) << " of " << numSeatsToBook << "\n";
            rowInput = getValidInt(
              "Enter row number (1-" + to_string(NUM_ROWS) + ", 0 to finish): ",
              0,
            NUM_ROWS
            );

            if (rowInput == 0) {
                cancelled = true;
                break;
            }

            seatInput = getValidInt(
                "Enter seat number (1-" + to_string(NUM_SEATS) + "): ",
                1,
                NUM_SEATS
            );

            if (!isValidRowSeat(rowInput, seatInput)) {
                cout << "Invalid row or seat number. Please try again.\n";
            }
            else if (seats[rowInput - 1][seatInput - 1] == SEAT_RESERVED) {
                char confirmReserved;
                cout << "This seat is already reserved. Complete payment for it now? (y/n): ";
                cin >> confirmReserved;

                if (confirmReserved == 'y' || confirmReserved == 'Y') {
                    seatConfirmed = true;
                } else {
                    cout << "Okay, please choose another seat.\n";
                }
            }
            else if (seats[rowInput - 1][seatInput - 1] == SEAT_TAKEN) {
                cout << "That seat is already been sold. \n" << "Please choose another seat.\n";
            }
            else {
                seatConfirmed = true;
            }
        }

        if (cancelled) {
            cout << "\nPayment cancelled. Reserved seats remain reserved.\n";
            cout << "Returning to menu.\n";
            return;
        }

        bookedRow[numBooked] = rowInput;
        bookedSeat[numBooked] = seatInput;
        numBooked++;
    }

    if (cancelled) {
        for (int i = 0; i < numBooked; i++) {
            seats[bookedRow[i] - 1][bookedSeat[i] - 1] = SEAT_AVAILABLE;
        }
        cout << "\nPayment cancelled. Returning to menu.\n";
        saveSeats(seats);
        return;
    }

    cout << "\nSeat(s) selected:\n";
    for (int i = 0; i < numBooked; i++) {
        cout << " Row " << bookedRow[i] << ", Seat " << bookedSeat[i] << "\n";
    }

    double subtotal = 0.0;
    for (int i = 0; i < numBooked; i++) {
        subtotal += getPriceForRow(priceList, bookedRow[i]);
    }

    processPayment(
        seats,
        promoList,
        bookingID,
        customerName,
        bookedRow,
        bookedSeat,
        numBooked,
        subtotal
    );
}


void processPayment(char seats[NUM_ROWS][NUM_SEATS],
                    const vector<PromoCode> &promoList,
                    string bookingID,
                    string customerName,
                    int bookedRow[],
                    int bookedSeat[],
                    int numBooked,
                    double subtotal) {
    cout << fixed << setprecision(2);
    cout << "Subtotal: RM " << subtotal << "\n";

    // ---- Promo code ----
    string promoCode = "-";
    double discountPercent = 0.0;
    char hasPromo;
    cout << "Do you have a promo code? (y/n):";
    cin >> hasPromo;

    if (hasPromo == 'y' || hasPromo == 'Y') {
        cout << "Enter promo code: ";
        cin >> promoCode;
        discountPercent = getDiscountPercent(promoList, promoCode);

        if (discountPercent > 0.0) {
            cout << "Promo code accepted: " << discountPercent << "% off.\n";
        } else {
            cout << "Invalid promo code - no discount applied.\n";
            promoCode = "-";
        }
    }

    double discountAmount = subtotal * (discountPercent / 100.0);
    double amountDue = subtotal - discountAmount;

    if (discountAmount > 0.0) {
        cout << "Discount  : -RM " << discountAmount << "\n";
    }
    cout << "Amount Due : RM " << amountDue << "\n";

    double amountPaid = getValidDouble("Enter amount paid by customer: RM ", amountDue);
    double change = amountPaid - amountDue;

    string method;
    cout << "Enter payment method (Cash/Card): ";
    cin >> method;

    cout << "Change: RM " << change << "\n";

    // Build a list of every seat booked, like "1-5;1-12;6-3"
    string seatList = "";
    for (int i = 0; i < numBooked; i++) {
        if (i > 0) {
            seatList += ";";
        }
        seatList += to_string(bookedRow[i]) + "-" + to_string(bookedSeat[i]);
    }

    ofstream outFile(PAYMENT_FILE, ios::app);

    outFile << bookingID << "," << customerName << ","
        << seatList << "," << numBooked << ","
        << amountDue << "," << amountPaid << "," << change << ","
        << method << "," << promoCode << "," << discountAmount << "\n";

    outFile.close();

    // Change all reserved seats to paid/sold
    for (int i = 0; i < numBooked; i++) {
    seats[bookedRow[i] - 1][bookedSeat[i] - 1] = SEAT_TAKEN;
    }

    // Save the updated seat status to seats.txt
    saveSeats(seats);

    cout << "Payment recorded successfully.\n";
}

void viewPaymentDetails() {
    string searchID;
    cout << "\nEnter Booking ID to search (0 to return to menu): ";
    cin.ignore();
    getline(cin, searchID);

    if (searchID == "0") {
        cout << "Returning to menu.\n";
        return;
    }

    ifstream inFile(PAYMENT_FILE);
    if (!inFile) {
        cout << "No payment records found.\n";
        return;
    }

    string line;
    bool found = false;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string id, name, rowStr, seatStr, dueStr, paidStr, changeStr, method, promoCode, discountStr;
        getline(ss, id, ',');

        if (id == searchID) {
            found = true;
            getline(ss, name, ',');
            getline(ss, rowStr, ',');
            getline(ss, seatStr, ',');
            getline(ss, dueStr, ',');
            getline(ss, paidStr, ',');
            getline(ss, changeStr, ',');
            getline(ss, method, ',');
            getline(ss, promoCode, ',');
            getline(ss, discountStr, ',');

            cout << "\n--------- PAYMENT DETAILS ---------\n";
            cout << "Booking ID     : " << id << "\n";
            cout << "Customer Name  : " << name << "\n";
            cout << "Seats Booked   : " << seatStr << " seat(s)\n";
            cout << "Seat List      : " << rowStr << "\n";
            cout << fixed << setprecision(2);
            if (!promoCode.empty() && promoCode != "-") {
                cout << "Promo Code      : " << promoCode << " (-RM "
                     << (discountStr.empty() ? 0.0 : stod(discountStr)) << ")\n";
            }
            cout << "Amount Due     : RM " << stod(dueStr) << "\n";
            cout << "Amount Paid    : RM " << stod(paidStr) << "\n";
            cout << "Change         : RM " << stod(changeStr) << "\n";
            cout << "Payment Method : " << method << "\n";
            break;
        }
    }
    inFile.close();

    if (!found) {
        cout << "No record found for Booking ID: " << searchID << "\n";
    }
}

void viewAllPayments() {
    ifstream inFile(PAYMENT_FILE);
    if (!inFile) {
        cout << "No payment records found.\n";
        return;
    }

    cout << "\n--------- ALL PAYMENT RECORDS ---------\n";
    string line;
    double totalRevenue = 0;
    int recordCount = 0;

    while (getline(inFile, line)) {
        stringstream ss(line);
        string id, name, rowStr, seatStr, dueStr, paidStr, changeStr, method;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, rowStr, ',');
        getline(ss, seatStr, ',');
        getline(ss, dueStr, ',');
        getline(ss, paidStr, ',');
        getline(ss, changeStr, ',');
        getline(ss, method, ',');

        cout << left << setw(10) << id << setw(15) << name
             << setw(6) << seatStr << "seat(s): " << setw(20) << rowStr
             << "RM " << fixed << setprecision(2) << stod(dueStr) << "\n";

        totalRevenue += stod(dueStr);
        recordCount++;
    }
    inFile.close();

    cout << "----------------------------------------\n";
    cout << "Total Bookings: " << recordCount << "\n";
    cout << "Total Revenue : RM " << fixed << setprecision(2) << totalRevenue << "\n";
}

// input validation
int getValidInt(string prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail() || value < minVal || value > maxVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between "
                 << minVal << " and " << maxVal << ".\n";
        } else {
            return value;
        }
    }
}

double getValidDouble(string prompt, double minVal) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail() || value < minVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Amount must be at least RM "
                 << fixed << setprecision(2) << minVal << ".\n";
        } else {
            return value;
        }
    }
}


// manager menu
void totalRevenue()
{
    viewAllPayments();
}

void seatsSold(char seats[NUM_ROWS][NUM_SEATS])
{
    int totalSold = 0;

    for (int row = 0; row < NUM_ROWS; row++)
    {
        for (int seat = 0; seat < NUM_SEATS; seat++)
        {
            if (seats[row][seat] == SEAT_TAKEN)
            {
                totalSold++;
            }
        }
    }

    cout << "\nTotal seats sold: " << totalSold << endl;
}

void seatsAvailable(char seats[NUM_ROWS][NUM_SEATS])
{
    int totalAvailable = 0;

    for (int row = 0; row < NUM_ROWS; row++)
    {
        for (int seat = 0; seat < NUM_SEATS; seat++)
        {
            if (seats[row][seat] == SEAT_AVAILABLE)
            {
                totalAvailable++;
            }
        }
    }
    cout << "\nTotal seats available: " << totalAvailable << endl;
}

void seatsReserved(char seats[NUM_ROWS][NUM_SEATS])
{
    int totalReserved = 0;

    for (int row = 0; row < NUM_ROWS; row++)
    {
        for (int seat = 0; seat < NUM_SEATS; seat++)
        {
            if (seats[row][seat] == SEAT_RESERVED)
            {
                totalReserved++;
            }
        }
    }

    cout << "Total seats reserved: " << totalReserved << endl;
}

void overallReport(char seats[NUM_ROWS][NUM_SEATS])
{
    cout << "\n========== OVERALL REPORT ==========\n";

    totalRevenue();
    seatsSold(seats);
    seatsReserved(seats);
    seatsAvailable(seats);

    cout << "====================================\n";
}

void manager(char seats[NUM_ROWS][NUM_SEATS])
{
    int choice;

    do
    {
        cout << "\n----- MANAGER MENU -----\n";
        cout << "1. Total Revenue\n";
        cout << "2. Seats Sold\n";
        cout << "3. Seats Available\n";
        cout << "4. Overall Report\n";
        cout << "5. Logout\n";

        choice = getValidInt
        (
            "Please enter your choice (1-5): ",
            1,
            5
        );

        switch (choice)
        {
            case 1:
                totalRevenue();
                break;

            case 2:
                seatsSold(seats);
                break;

            case 3:
                seatsAvailable(seats);
                break;

            case 4:
                overallReport(seats);
                break;

            case 5:
                cout << "Logging out...\n";
                break;
        }

    } while (choice != 5);
}
