# friends-server
A network program written in C where clients can connect and issue friends commands.

## Table of Contents
1. [Description](#description)
2. [Installation](#installation)
3. [Usage](#usage)
4. [Commands](#commands)

## Description
- The motivation behind building this project was to develop a simple messaging tool for efficient and user-friendly 
communication among friends.
- By eliminating the unnecessary complexities and features of other platforms, this project aimed to streamline the 
experience and provide a lightweight, portable platform.
- Additionally, it removes the need for an extensive sign-up process and profile set up, allowing users to quickly 
start communicating with their friends.
- From this project, I learned how to a create a server in C with sockets, which allows any number of users to connect.
- I also learned how to juggle many client connections, and how to prevent deadlock when attempting to check multiple 
sockets for data.

## Installation
1. Clone the repository
2. Run make on the root directory
3. Run the executable friend_server


## Usage
1. Starting the server
2. Compile the project, see [installation](#Installation)
3. Run the executable friend_server
4. The default port is 56524
5. The server is up and running!

## Commands
1. quit <br/>
Disconnects the client from the server<br/>
Usage: $quit


2. list_users<br/>
Lists all the users that have registered to this server<br/>
Usage: $list_users


3. make_friends<br/>
Makes a friendship between the currrent user and the **username**<br/>
Usage: $make_friends **username**

4. post<br/>
Posts a **message** on the **target**'s profile<br/>
Usage: post **target** **message**

5. profile<br/>
Shows the profile of the **username**<br/>
Usage: profile **username**
