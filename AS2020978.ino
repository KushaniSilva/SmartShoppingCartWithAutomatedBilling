#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>

#define RST_PIN 9
#define SS_PIN 10 
#define GREEN_LED_PIN 4
#define RED_LED_PIN 5
#define ADD_BUTTON_PIN 2
#define REMOVE_BUTTON_PIN 7
#define FINISH_BUTTON_PIN 6

byte taguid[5][4]={{0x13,0x0F,0XEE,0XB9},{0xA9,0x4C,0X69,0X98},{0x09,0x03,0X6E,0X98},{0x29,0x28,0XE3,0X97},{0xA3,0x87,0X01,0X4D}};
int cartVal= 0;
int elements[20][2];
String invName[4]={"Herbaltea-Rs.440","Noodles-Rs.355","Biscuit-Rs.120","Milkpkt-Rs.120"};
int detail[5][3]={{440,0,0},{355,0,0},{120,0,0},{120,0,0},{0,0,0}};
int addButton=0;
int removeButton=0;
int finishButton=0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

//compare two RFID UIDs
bool compareUID(byte* uid1, byte* uid2) {
  for (int i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ADD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(REMOVE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FINISH_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  lcd.setCursor(3,0);
  lcd.print("WELCOME!");
  lcd.setCursor(0,1);
  lcd.print("HAPPY SHOPPING!");
  delay(3000);
}

void loop() {
  if (digitalRead(ADD_BUTTON_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("ADD ITEMS TO");
    lcd.setCursor(6,1);
    lcd.print("CART=)");
    delay(2000);
    addItemToCart();
  }

  if (digitalRead(REMOVE_BUTTON_PIN) == LOW) {
    lcd.clear();
    removeItemFromCart();
  }

  if (digitalRead(FINISH_BUTTON_PIN) == LOW) {
    lcd.clear();
    lcd.print("Thank you!");
    lcd.setCursor(0, 1);
    lcd.print("Total:");
    float totalAmount = calculateTotalPrice(); // Calculate and display total amount
    lcd.print("Rs.");
    lcd.print(totalAmount);
    delay(2000);
  }
}

void addItemToCart() {
  lcd.clear();
  lcd.print("Scan RFID to add item");

  bool addingItems = true;  // Flag to indicate whether still adding items
  while (addingItems) {
    // Check if buttons are pressed
    if (digitalRead(REMOVE_BUTTON_PIN) == LOW || digitalRead(FINISH_BUTTON_PIN) == LOW) {
      lcd.clear();
      lcd.print("Exiting...");
      delay(1000);
      addingItems = false;  // Exit the loop when a button is pressed
      break;
    }

    // Read RFID tag
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte tagUID[4];

      //Extracts the UID of the scanned RFID tag
      for (int i = 0; i < 4; i++) {
        tagUID[i] = mfrc522.uid.uidByte[i];
      }

      // Check if the scanned RFID tag matches any in the list
      int itemIndex = -1;
      for (int i = 0; i < 5; i++) {
        if (compareUID(tagUID, taguid[i])) {
          itemIndex = i;
          break;
        }
      }

      if (itemIndex != -1) {// if item found, update cart and display information
        cartVal++;
        elements[cartVal - 1][0] = itemIndex;
        elements[cartVal - 1][1]++;

        lcd.clear();
        lcd.print("Added: ");
        lcd.print(invName[itemIndex]);
        lcd.setCursor(0, 1);
        lcd.print("Price: Rs.");
        lcd.print(detail[itemIndex][0]);

        // Turn on green LED
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(2000);
        digitalWrite(GREEN_LED_PIN, LOW);

        // Display total price
        lcd.clear();
        lcd.print("Total: Rs.");
        lcd.print(calculateTotalPrice());
        delay(2000);

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      } else {
        // Display error message for unrecognized RFID
        lcd.clear();
        lcd.print("Unrecognized RFID");
        delay(2000);

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      }
    }
  }
}

void removeItemFromCart() {
  lcd.clear();
  lcd.print("Scan RFID to ");
  lcd.setCursor(0, 1);
  lcd.print("remove item");

  bool cardRemoved = false;

  while (!cardRemoved) {
    // Check if buttons are pressed
    if (digitalRead(ADD_BUTTON_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("ADD ITEMS TO");
      lcd.setCursor(6, 1);
      lcd.print("CART=)");
      delay(2000);
      addItemToCart();
    }

    if (digitalRead(FINISH_BUTTON_PIN) == LOW) {
      lcd.clear();
      lcd.print("Thank you!");
      // Calculate and display total amount
      lcd.setCursor(0, 1);
      lcd.print("Total: Rs.");
      lcd.print(calculateTotalPrice());
      delay(2000);
      return;  // Exit the removeItemFromCart function
    }

    // Read RFID tag
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte tagUID[4];

      for (int i = 0; i < 4; i++) {
        tagUID[i] = mfrc522.uid.uidByte[i];
      }

      // Check if the scanned RFID tag matches any in the list
      int itemIndex = -1;
      for (int i = 0; i < 20; i++) {
        if (compareUID(tagUID, taguid[elements[i][0]])) {
          itemIndex = i;
          break;
        }
      }

      // Exit the loop immediately after reading a card
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();

      // Check if the item is in the cart before attemptng to remove it
      if (itemIndex != -1 && elements[itemIndex][1] > 0) {
        // Item is in the cart, update cart and display removal message
        cartVal--;
        elements[itemIndex][1]--;

        lcd.clear();
        lcd.print("Removed: ");
        lcd.print(invName[elements[itemIndex][0]]);
        lcd.setCursor(0, 1);
        lcd.print("Price: Rs.");
        lcd.print(detail[elements[itemIndex][0]][0]);

        // Turn on red LED
        digitalWrite(RED_LED_PIN, HIGH);
        delay(2000);
        digitalWrite(RED_LED_PIN, LOW);

        // Display total price
        lcd.clear();
        lcd.print("Total: Rs.");
        lcd.print(calculateTotalPrice());
        delay(2000);

        cardRemoved = true;
      } else {
        // Display error message for unrecognized RFID or item not in the cart
        lcd.clear();
        lcd.print("Item not found");
        delay(2000);
        // Continue the loop to wait for another RFID scan
      }
    }
  }
}

float calculateTotalPrice() {
  float totalPrice = 0;

  // Iterate through elements array to calculate total price
  for (int i = 0; i < 20; i++) {
    int itemIndex = elements[i][0];
    int quantity = elements[i][1];

    if (quantity > 0) {
   // Calculate the total price for added items
      totalPrice += detail[itemIndex][0] * quantity;
    }
  }

  // Display removed items
  for (int i = 0; i < 5; i++) {
    int quantityRemoved = detail[i][1];

    if (quantityRemoved > 0) {
      // Subtract the total price for removed items
      totalPrice -= detail[i][0] * quantityRemoved;
    }
  }

  return totalPrice;
}
