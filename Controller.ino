#define BTN_PIN A5

#define ROW_1 10
#define ROW_2 11
#define ROW_3 12
#define ROW_4 13
#define ROW_5 A1
#define ROW_6 A2
#define ROW_7 A3
#define ROW_8 A0

#define COL_1 2
#define COL_2 3
#define COL_3 4
#define COL_4 5
#define COL_5 6
#define COL_6 7
#define COL_7 8
#define COL_8 9

const byte rows[] = {
  ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8,
};

// The display buffer
// (1 = ON, 0 = OFF)
// Patterns for numbers 0 to 10
byte numbers[][8] = {
  {
    B00011100,
    B00100010,
    B00100010,
    B00100010,
    B00100010,
    B00100010,
    B00100010,
    B00011100
  },
  {
    B00001000,
    B00001100,
    B00001000,
    B00001000,
    B00001000,
    B00001000,
    B00001000,
    B00011100
  },
  {
    B00011100,
    B00100010,
    B00100000,
    B00100000,
    B00010000,
    B00001000,
    B00000100,
    B00111110
  },
  {
    B00011100,
    B00100010,
    B00100000,
    B00011000,
    B00100000,
    B00100000,
    B00100010,
    B00011100
  },
  {
    B00100000,
    B00110000,
    B00101000,
    B00100100,
    B00100010,
    B00111110,
    B00100000,
    B00100000
  },
  {
    B00111110,
    B00000010,
    B00000010,
    B00111110,
    B01000000,
    B01000000,
    B01000100,
    B00111000
  },
  {
    B00011100,
    B00100010,
    B00000010,
    B00011110,
    B00100010,
    B00100010,
    B00100010,
    B00011100
  },
  {
    B00111110,
    B00100000,
    B00100000,
    B00010000,
    B00001000,
    B00000100,
    B00000100,
    B00000100
  },
  {
    B00011100,
    B00100010,
    B00100010,
    B00011100,
    B00100010,
    B00100010,
    B00100010,
    B00011100
  },
  {
    B00011100,
    B00100010,
    B00100010,
    B00100010,
    B00111100,
    B00100000,
    B00100010,
    B00011100
  },
};
byte number10[] = {
  B01100000,
  B10010010,
  B10010011,
  B10010010,
  B10010010,
  B10010010,
  B01100111,
  B00000000
};

String incomingData = ""; // To store incoming data

void setup() {
  // Open serial port
  Serial.begin(9600);
  
  // Set all used pins to OUTPUT
  for (byte i = 2; i <= 13; i++)
    pinMode(i, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); // Set the button pin as INPUT_PULLUP
}

int currentDisplayedNumber = -1; // Initialize with an invalid number
unsigned long displayStartTime = 0; // Variable to store the display start time
const unsigned long displayDuration = 1000; // Adjust this value for the desired display duration in milliseconds

void loop() {
  // Check for incoming data
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'P') {
      // Received 'P', send a message back to C++ project
      Serial.println("P");
    } else if (c == 'T') {
      // Received 'T', display number 10
      displayNumber(10);
      currentDisplayedNumber = 10; // Update the current displayed number
      displayStartTime = millis(); // Record the display start time
    } else if (c >= '0' && c <= '9') {
      // Received a digit, display it
      int digit = c - '0';
      if (digit != currentDisplayedNumber) {
        displayNumber(digit);
        currentDisplayedNumber = digit; // Update the current displayed number
        displayStartTime = millis(); // Record the display start time
      }
    }
  }
  
  // Check if the button is pressed
  if (digitalRead(BTN_PIN) == HIGH) {
    // Button is pressed, send 'P' to the C++ project
    Serial.println("P");
    delay(1000); // Delay to debounce the button
  }
  
  // Check if it's time to turn off the display
  if (currentDisplayedNumber != -1 && millis() - displayStartTime >= displayDuration) {
    displayNumber(-1); // Turn off the display
    currentDisplayedNumber = -1; // Reset the current displayed number
  }
}

void displayNumber(int digit) {
  if (digit >= 0 && digit <= 9) {
    drawScreen(numbers[digit]);
  } else if (digit == 10) {
    drawScreen(number10); // Display number 10
  }
}

void drawScreen(const byte buffer[]) {
  // Turn on each row in series
  for (byte i = 0; i < 8; i++) {
    setColumns(buffer[i]); // Set columns for this specific row
    
    digitalWrite(rows[i], HIGH);
    delay(1); // Set this to 50 or 100 if you want to see the multiplexing effect!
    digitalWrite(rows[i], LOW);
  }
}

void setColumns(byte b) {
  digitalWrite(COL_1, (~b >> 0) & 0x01);
  digitalWrite(COL_2, (~b >> 1) & 0x01);
  digitalWrite(COL_3, (~b >> 2) & 0x01);
  digitalWrite(COL_4, (~b >> 3) & 0x01);
  digitalWrite(COL_5, (~b >> 4) & 0x01);
  digitalWrite(COL_6, (~b >> 5) & 0x01);
  digitalWrite(COL_7, (~b >> 6) & 0x01);
  digitalWrite(COL_8, (~b >> 7) & 0x01);
}
