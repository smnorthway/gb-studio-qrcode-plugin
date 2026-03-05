The usual "software not warrented to be suitable for the intended purpose".

# gb-studio-qrcode-plugin
This engine plugin generates in-game qrcodes in GB Studio 4.2.2. It is installed by 
copying the "via_norte" folder and all subfolders into your game's "plugins" folder. 
 Two new events will showup in your "Add Event" dialogue. You can find them by searching on "qr".

 You can also check them out by downloading the "qr demo" folder and subfolders. It
 contains a one scene game that lets you enter a player name and then create a usable qr code.


i created this plugin to connect to a "global" leaderboard website I setup
for a GB game I made for my grandchildren.

This is an engine plugin that generates and displays a qrcode from a 
display-dialogue-like input. i used it to display the leaderboard url. 
Arguments in the url included a username and score.


The plugin also includes a hash calculator. The output of the hash are stored in 5 global variables. 
I included a hash of the 
username and score as an argument in the leaderboard url. The server side of the leaderboard 
website recalculated the hash to ensure the arguments haven't changed. I have 
included a js implementaion of the same hash calculation.

## Display QR Code Event
The only input is in a dialogue box. Global variables can be included. 
They will be expanded as in a usual Display Dialogue event. 
The number of characters is limited to about 100. 
The qr code remains displayed until the "A" button is pressed.

<img width="300" height="290" alt="image" src="https://github.com/user-attachments/assets/704d455b-0890-449f-810d-e7af6997e76a" />


## Calculate QR Hash Event
The characters for hashing are entered into a dialogue box. The results of the hash are
stored in 5 Global variables. These 5 variables need to be entered in the event menu.
On completion these variables will contain the ascii values of the hash characters. 

The hash can be used by a companion website to ensure the url arguments have not 
been altered. This is accomplished by the website calculating a hash using the same 
rules as the game. If the two hashes match, all is well.

A couple of notes on using the hash. Make sure the website calculates and 
compares the hashes on the server side and not the client side. Otherwise, the hashing rules are exposed. 
The hash is based on a relatively simple XOR-ROTATED-CHECKSUM, just intended to discourage casual hacking. 
You should include a private key within the characters to be hashed. What 
the key is and how 
the key is used in the hashing rules should only be exposed in the game's code and the server side of the 
companion website.

I have included an html/js implementaion of the hash for testing and as a basis for website implementation.




