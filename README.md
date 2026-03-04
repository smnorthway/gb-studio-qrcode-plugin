# gb-studio-qrcode-plugin
This engine plugin generates in-game qrcodes. 


i created this plugin to connect to a "global" leaderboard website I setup
for a GB game I made for my grandkids.

This is an engine plugin that generates and displays a qrcode from a 
display-dialogue-like input. i used it to display the leaderboard url. 
username and score were included as arguments in the url.


The plugin also includes a hash calculator. I included a hash of the 
username and score as an argument in the leaderboard url. The server side of the leaderboard 
website recalculates the hash to ensure the arguments haven't changed.
