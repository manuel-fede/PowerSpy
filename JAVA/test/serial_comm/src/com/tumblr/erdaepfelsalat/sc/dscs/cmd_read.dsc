The read command is used to create a listening thread for incoming data.
This thread will, regardless of what the user does at the moment post incoming
data. It has two states: start and stop. Which do the respective thing.
There may only be 1 thread open at a time, a second one would not make sense.
requirements: An active connection.
arguments: <state>