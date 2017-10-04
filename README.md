# ECE3140-RealTimeStocks

## Introduction 
This project is a real time display of stock data. It has three major components:
an FTP connection that receives text data from a python script that queries Yahoo Finance and
writes a .txt file, data storage and reading from a microSD card in the board, displaying the text
on an 8x8 LED matrix. We were able to design and develop an LED matrix that displays stock
data that the FRDM board can retrieve in real time through the FTP.
The basic workflow is simple: Use a python script to generate data files, send the data files to
the board via FTP, then the stock data will continuously scroll and display the data. Any time
you wish to change what is currently being displayed, simply generate and upload another data
file, and the scrolling display will be updated.

## System Diagram
![System Diagram](https://github.com/RohitBandaru/ECE3140-RealTimeStocks/blob/master/Screen%20Shot%202017-10-04%20at%2012.58.58%20PM.png)
The non-microcontroller is a laptop that runs the python script that generates the .txt file that
stores the stock data. This file is sent through the ethernet FTP connection to be stored in the
MicroSD card. Changing the data on the card alerts the matrix control to change the display
text.
Our alert system works like an interrupt that sets a flag indicating that a new data file has been
uploaded. So, when the led matrix reaches the end of a loop display cycle, our system knows to
read the new data file and update the display. This is superior to polling because polling would
entail reading the data file after every loop display cycle in case the data has been changed.

## Hardware Description
- MicroSD card
- Ethernet cable with USB adapter
- 8x8 Linksprite LED matrix
- 16x8 Adafruit LED matrix (not in use)
- Jumper cables

The MicroSD card was inserted into the FRDM board. The LED matrix is connected to the board
using jumper cables. The ports are as follows: CLK, CS, DIN, GND, VCC connected to ports
PTD1, PTD0, PTD2, GND, 5V respectively. The board is connected to the computer
(non-microcontroller component) using an ethernet cable with a USB adapter. This connection
is how the data is communicated through FTP. The connections are made as shown in the
system diagram.

## Software Description
Our main function is split into two parts, set up and a while loop. In the setup, the ports
are configured and the file system is initialized.
The data is communicated to the LED matrix in a Serial-Data format. In order to write a
byte of data, each bit is looped through and between the CLK set to LOW and the CLK being
set to high, the bit is written to DIN. This is done in the write_byte function.
![Timing Diagram](https://github.com/RohitBandaru/ECE3140-RealTimeStocks/blob/master/Screen%20Shot%202017-10-04%20at%201.00.14%20PM.png)
The LED matrix is updated by writing a byte to an address, which is a column on the
display. The write function used to write a column on the matrix using the serial data format, the
address byte is written and then the data byte.
For the LED matrix the main global variables are current_text (the text being displayed),
update_occurred (boolean that is set to 1 when text needs to updated), and STATE, which is
the current display on the LED matrix.
For the data communication part, we had two main components, the FTP connection,
and the MicroSD card file system. The file system was implemented using MDK Middleware.
We used a helper function to format the drive.
On the non-microcontroller side, a python script is run to write a .txt file. The data is
pulled from Yahoo Finance using the yahoo-finance python package. An FTP is run on the
computer through the command prompt. Once a connection is established the text file is sent to
the board from the computer.
When data is received through the FTP, the netFTPs_Notify function is called and
update_occurred is set to 1. When this global variable is set to one, a code block in the
display_stocks function (which is called in the main while loop) is run. This code opens the file in
the MicroSD card and updates the current_text global variable. However, this is designed to
finish displaying the current text before displaying the new text.
In the main while loop, there are two functions: net_main and display_stocks.
Display_stocks checks if an update has occurred and performs the update as described earlier.
It then calls the display_string function, which deals directly with the LED matrix pins and
displays the scrolling text.

## Testing
We tested each component of our project separately, and then all together. We first
wrote the code to control the LED matrix. Once it was displaying single characters, we wrote
code to expand it to scrolling text which would accommodate our stock data display.
To test our FTP connection and file system, we sent false data to the board and verified
that the data was written. Often in coding we used debugging tools in Keil uvision like watches.
Results and Challenges:
We achieved the core of what we proposed but made major changes in implementation.
To begin with, we switched from a 16x8 I2C LED matrix to 8x8 GPIO LED Matrix. In office
hours, Ian noticed that our 16x8 board was really hot, and said it was very likely we fried the
board due to our mistakes in wiring. We then decided to buy an 8x8 because it was cheaper
and we might have better luck using GPIO.
A more significant hurdle for us was networking. We spent significant time trying to get
the FRDM board to connect directly to the internet. We tried various approaches including trying
to set up an HTTP server on the board. We were unable to get the board to connect directly to
the internet but instead were able use an FTP connection and SD card to pull data from a
computer. It is far simpler for a computer to connect to the internet and send data to the board.
If we were to repeat this project, we would spend time assessing different approaches to
implementing our high level functionalities and choose an implementation that is more feasible,
instead of spending many hours on implementations that do not work.

## References
Used helper functions as commented in code
https://www.keil.com/pack/doc/mw/Network/html/_f_t_p__server__example.html
https://www.keil.com/pack/doc/mw/FileSystem/html/fs_examples.html
http://linksprite.com/wiki/index.php5?title=LED_Matrix_Kit
https://pypi.python.org/pypi/yahoo-finance
Past references:
http://www.keil.com/appnotes/files/apnt_271.pdf
https://learn.adafruit.com/adafruit-led-backpack/0-8-8x8-matrix
