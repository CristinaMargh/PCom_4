## Margheanu Cristina-Andreea 323CAa - PCom_4 Homework - Web Client

* I used lab 9 as a starting point in my implementation, the files buffer, helpers, being taken integral from there. The actual implementation was done in the client and request files. 

* The code in request contains messages of type POST, GET, DELETE, send to the server. These will be used in the client.c (for example GET to obtain a certain book, DELETE to remove it from the system).

* In client.c the program's flow starts in the main function where I centralized the
commands. Depending on the input from stdin, I perform the corresponding function. 
At the begining of the program I declared the variable in_system, which I will use during the code's flow to save the loged user to be able to work with him in future functions. I also declared the token there too.

* Each function starts by opening the connection, checking its correctness and ends by closing it.
For the login and register functions, we also check if the username or password contain spaces in them.
We continue with sending the message to the server and checking the response. If the answer exists, an error or success message is displayed as the case may be.
Analysis of messages for part of the functions.
- login : If the login was successful, we will retain the data of the logged in person, i.e. the message after connect.sid.
- enter_library: If we managed to enter the library, we also keep the token that we prefix with the word Bearer.
- Books functions : In the case of error messages when we want to get a certain book, add or delete it, they are obtained from the response from the server. For the function of obtaining a certain book, we use JSON_Array,
   we find their numbers, go through them and display for each the id and the title in the required format.

At the almost end of the session, in the logout function, we release all used resources.
