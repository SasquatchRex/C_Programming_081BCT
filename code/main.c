#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <qrencode.h>

// Initialization of Authentication
int Login();
void Logout();
int Signup();

// Initialization of Pages
void mainscreen();
void Dashboard();
void CheckStocks();
void AddStocks();
void SellStocks();


int update_stock(sqlite3 *db, const char *product_name, int purchase_quantity);

// Main Function

void main()
{
    mainscreen();
}

// Pages

// Callback function to handle query results
int callback(void *NotUsed, int argc, char **argv, char **azColName)
{

    printf("%-5d | %-20s | %-10d | %-10d\n", atoi(argv[0]), argv[1], atoi(argv[2]), atoi(argv[3]));

    return 0;
}

void CheckStocks()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open("users.db", &db);
    char choice[1];
    system("clear");
    printf("Check Stocks :\n");
    printf("SN \t Name \t\t\t\t Quantity \n");

    // printf("No stocks found. \n");
    const char *sql = "SELECT * FROM Stocks;";
    char *errMsg = NULL;
    rc = sqlite3_exec(db, sql, callback, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error retriving data: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    printf("Press d to go to dashboard \n");
    printf("Press l to logout \n");

    scanf("%c", choice);
    switch (choice[0])
    {
    case 'd':
        Dashboard();
    case 'l':
        Logout();

    default:
        CheckStocks();
    }
}
void AddStocks()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open("users.db", &db);
    char name[50];
    int choice;
    int quantity, unit_price;
    system("clear");
    printf("Add Stocks : \n");
    printf("Enter Item Name : ");
    scanf("%s", name);
    printf("Enter Quantity : ");
    scanf("%d", &quantity);
    printf("Enter unit Price : ");
    scanf("%d", &unit_price);

    const char *sql = "CREATE TABLE IF NOT EXISTS Stocks("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "Name TEXT UNIQUE NOT NULL,"
                      "Quantity INTEGER NOT NULL,"
                      "UnitPrice INTEGER NOT NULL);";
    char *errMsg = NULL;
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error creating table: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
        printf("Table checked/created successfully.\n");
        const char *sql = "INSERT INTO Stocks (Name,Quantity,UnitPrice) VALUES (?,?,?)";
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, quantity);
        sqlite3_bind_int(stmt, 3, unit_price);

        // Executing SQL statement
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE)
        {
            printf("Data Added Sucessfully!\n");
        }
        else
        {
            printf("Error: %s\n", sqlite3_errmsg(db));
        }

        // Cleanup
        sqlite3_finalize(stmt);
    }

    printf("1. Dashboard \n");
    printf("2. Add Stocks \n");

    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        /* code */
        Dashboard();
    case 2:
        AddStocks();

    default:
        Dashboard();
        printf("Default");
    }
}

struct Stocks_billing
{
    char name[50];
    int quantity, unitprice;
};

void SellStocks()
{
    struct Stocks_billing Stocks_billing[20];
    system("clear");
    sqlite3 *db;
    sqlite3_stmt *stmt1;
    sqlite3_stmt *stmt2;
    // sqlite3_stmt *stmt3; 
    int num, quantity;
    char name[50];
    int choice;
    char qr_data[100];  // Buffer for formatted string
    // const char *sql = "UPDATE Stocks SET Quantity = Quantity - ? WHERE Name = ? RETURNING UnitPrice";
    const char *select_sql = "SELECT UnitPrice FROM Stocks WHERE Name = ?";
    const char *update_sql = "UPDATE Stocks SET Quantity = Quantity - ? WHERE Name = ?";
    // const char *check_sql = "SELECT Quantity FROM Stocks WHERE Name = ?";


    int error = 0;

    // Open the database
    if (sqlite3_open("users.db", &db) != SQLITE_OK)
    {
        printf("Error opening database: %s\n", sqlite3_errmsg(db));
    }


    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt1, NULL) != SQLITE_OK) {
        printf("Error preparing select statement: %s\n", sqlite3_errmsg(db));
    
    }

    if (sqlite3_prepare_v2(db, update_sql, -1, &stmt2, NULL) != SQLITE_OK) {
        printf("Error preparing update statement: %s\n", sqlite3_errmsg(db));
    
    }

    // if(sqlite3_prepare_v2(db,check_sql,-1,&stmt3,0) == SQLITE_OK){
    //     sqlite3_bind_text(stmt3, 1, product_name, -1, SQLITE_STATIC);
        
    //     if (sqlite3_step(stmt) == SQLITE_ROW) {
    //         available_quantity = sqlite3_column_int(stmt, 0);
    //     }
    //     sqlite3_finalize(stmt);
    // }


    // Prepare the SQL statement
    // if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    // {
    //     printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
    //     sqlite3_close(db);
    // }



    printf("Sell Stocks : \n");
    printf("\nEnter the number of stocks to be sold : ");
    scanf("%d", &num);

    for(int i=1;i<=num;i++){
        printf("\nEnter the name of stock : ");
        scanf("%s",name);
        printf("\n Quantity");
        scanf("%d",&quantity);

        sqlite3_bind_text(stmt1, 1, name, -1, SQLITE_STATIC);

        int unit_price = 0;
        if (sqlite3_step(stmt1) == SQLITE_ROW) {
            unit_price = sqlite3_column_double(stmt1, 0);
        } else {
            printf("Item not found in stock.\n");
            sqlite3_finalize(stmt1);
            continue;
        }
        sqlite3_finalize(stmt1); 


        // sqlite3_bind_int(stmt2, 1, quantity);
        // sqlite3_bind_text(stmt2, 2, name, -1, SQLITE_STATIC);

        // if (sqlite3_step(stmt2) != SQLITE_DONE) {
        //     printf("Error updating record: %s\n", sqlite3_errmsg(db));
        // } else {
        //     printf("Stock updated successfully.\n");
        // }

        // sqlite3_finalize(stmt2);



        error = update_stock(db, name, quantity);




    

        strcpy(Stocks_billing[i].name,name);
        Stocks_billing[i].quantity = quantity;
        Stocks_billing[i].unitprice = unit_price;

        // printf("%s \t %d \t %d",name,quantity,unit_price);
    }

    sqlite3_close(db);

    system("clear");
    int amount =0;
    
    
    if(error == 0){
        printf("Bill :  \n");
        printf("------------------------------------------------------------------------\n");
        printf("%-5s | %-20s | %-10s | %-10s | %-20s\n","SN","Name","Quantity","Rate","Amount");
        for(int i=1;i<=num;i++){
            amount += Stocks_billing[i].quantity*Stocks_billing[i].unitprice;
            printf("%-5d | %-20s | %-10d | %-10d | %-20d\n",i,Stocks_billing[i].name,Stocks_billing[i].quantity,Stocks_billing[i].unitprice,Stocks_billing[i].quantity*Stocks_billing[i].unitprice);
        
        }
        printf("------------------------------------------------------------------------\n");
        printf("%-5s | %-20s | %-10s | %-10s | %-20d\n","","","","Total",amount);
        printf("------------------------------------------------------------------------\n\n\n");
        sprintf(qr_data, "{'price': %d, 'id': '10010'}", amount);

        QRcode *qrcode = QRcode_encodeString(qr_data, 1, QR_ECLEVEL_L, QR_MODE_8, 0);
        if (qrcode) {
            for (int y = 0; y < qrcode->width; y++) {
                for (int x = 0; x < qrcode->width; x++) {
                    printf(qrcode->data[y * qrcode->width + x] & 1 ? "██" : "  ");
                }
                printf("\n");
            }
            QRcode_free(qrcode);
        } else {
            printf("Failed to generate QR Code\n");
        }

        printf("\n\n\n---------------------------QR CODE --------------------------------------\n");
        



        goto_default:

        printf("1. Sell Stocks\n2.Dashboard\n");
        printf("Enter choice : ");
        scanf("%d",&choice);

        switch (choice)
        {
        case 1:
            SellStocks();
            break;
        case 2:
            Dashboard();
            break;
        
        default:
            goto goto_default;
            break;
        }


        Dashboard();
    }
    else{
        system("clear");
        printf("\nInvalid quantity\n");
        sleep(2);
        system("clear");
        SellStocks();
    }
    


  
}

int update_stock(sqlite3 *db, const char *product_name, int purchase_quantity) {
    sqlite3_stmt *stmt;
    int available_quantity = 0;

    // Step 1: Check current stock
    const char *check_sql = "SELECT Quantity FROM Stocks WHERE Name = ?";
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, product_name, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            available_quantity = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error checking stock: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Step 2: Validate stock
    if (purchase_quantity > available_quantity) {
        printf("Error: Not enough stock! Available: %d, Requested: %d\n", available_quantity, purchase_quantity);
        return -1;
    }

    // Step 3: Update stock
    const char *update_sql = "UPDATE Stocks SET Quantity = Quantity - ? WHERE Name = ?";
    if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, purchase_quantity);
        sqlite3_bind_text(stmt, 2, product_name, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Stock updated successfully!\n");
            sqlite3_finalize(stmt);
            return 0;
        }
    }

    printf("Error updating stock: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
}

void Logout()
{
    mainscreen();
}

void Dashboard()
{
    system("clear");
    int choice;
    printf("Welcome to Dashboard of Recordex! \n");
    printf(" 1. Check Stocks \n 2. Add Stocks \n 3. Sell Stocks \n 4. Logout \n");
    printf("Enter your choice ( number )");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        CheckStocks();
        break;
    case 2:
        AddStocks();
        break;
    case 3:
        SellStocks();
        break;
    case 4:
        Logout();
        break;

    default:
        printf("Enter correct choice!");
        sleep(2);
        Dashboard();
        break;
    }
}
void mainscreen()
{
    // system("clear");
    int choice;
    printf("Welcome to Recordex \n");
    printf("1. Login \n2. Signup \n");
    printf("Enter your choice ( number ) : ");
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        Login();
        break;
    case 2:
        Signup();
        break;
    default:
        printf("Select correct choice !");
        mainscreen();
    }
}

// Authentication

// Function to hash password using SHA-256 and convert it to a hexadecimal string
void sha256(const char *input, char *output)
{
    unsigned char hash[32]; // SHA-256 produces a 32-byte binary hash
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    if (mdctx == NULL)
    {
        printf("EVP_MD_CTX_new failed\n");
        return;
    }

    // Initialize SHA-256 context
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
    {
        printf("EVP_DigestInit_ex failed\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // Update the hash with input
    if (EVP_DigestUpdate(mdctx, input, strlen(input)) != 1)
    {
        printf("EVP_DigestUpdate failed\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // Finalize and store the hash
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1)
    {
        printf("EVP_DigestFinal_ex failed\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // Convert hash to a hexadecimal string
    for (int i = 0; i < hash_len; i++)
    {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0'; // Null-terminate the string

    // Clean up
    EVP_MD_CTX_free(mdctx);
}

void createTable(sqlite3 *db)
{
    const char *sql = "CREATE TABLE IF NOT EXISTS Users("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT UNIQUE NOT NULL,"
                      "password TEXT NOT NULL);";
    char *errMsg = NULL;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("Error creating table: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
        printf("Table checked/created successfully.\n");
    }
}

void addUser(sqlite3 *db, const char *username, const char *password)
{
    sqlite3_stmt *stmt;
    char hashed_password[65];

    // Hashing the password before storing
    sha256(password, hashed_password);

    const char *sql = "INSERT INTO Users (username,password) VALUES (?,?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);

    // Executing SQL statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
    {
        printf("User registered successfully!\n");
    }
    else
    {
        printf("Error: %s\n", sqlite3_errmsg(db));
    }

    // Cleanup
    sqlite3_finalize(stmt);
}

int Login()
{
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc = sqlite3_open("users.db", &db);
    char username[10], password[10];
    char hashed_password[65];
    system("clear");
    printf("Welcome to Login Screen \n");
    printf("Enter the username : ");
    scanf("%9s", username);
    printf("\nEnter Password : ");
    scanf("%9s", password);
    sha256(password, hashed_password);

    printf("Hased Password : %s", hashed_password);

    // preparation for sql query
    const char *sql = "SELECT * FROM Users WHERE username=? AND password=?";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);

    // login logic
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        printf("\nLogin successful!");
        Dashboard();
    }
    else
    {
        printf("Invalid credentials.\n");
        // printf("%s",stmt);
    }

    // cleanup

    return 0;
}

int Signup()
{
    sqlite3 *db;
    int rc = sqlite3_open("users.db", &db);
    char username[10], password1[10], password2[10];
    // unsigned char hash[EVP_MAX_MD_SIZE];
    system("clear");
    printf("Welcome to Signup Screen \n");
    printf("Enter the username : ");
    scanf("%9s", username);
    printf("\nSet Password : ");
    scanf("%9s", password1);
    printf("\nEnter Password again : ");
    scanf("%9s", password2);
    if (strcmp(password1, password2) == 0)
    {
        if (rc)
        {
            printf("Can't open database: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            createTable(db);

            addUser(db, username, password1);
        }

        sleep(2);
        mainscreen();
    }
    else
    {
        printf("Password matching failed! \n");
        sleep(2);
        Signup();
    }
    return 0;
}
