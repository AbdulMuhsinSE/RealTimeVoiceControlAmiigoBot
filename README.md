Jarvis Control Guide v1.0.0
===========================

Jarvis is a voice control interface that allows the user to control two
robots through the use of voice at the same times. As currently
implemented the robots that are being controlled are the ones with the
following IPs:

-   10.0.126.14

-   10.0.126.15

Please be sure that both robots are on before running the program as the
program will terminate execution if either of the robots are unable to
continue.

To compile the multiplerobot.cpp:

g++ -O3 -o Robot.o multipleRobots.cpp \`pkg-config --cflags --libs
pocketsphinx sphinxbase\` -std=c++11 -fPIC -I/usr/local/Aria/include
-L/usr/local/Aria/lib -lAria -lpthread -ldl -lrt

This will produce a Robot.o file that can be run using the following
command:

./Robot.o

The corpus for control words:

-   JARVIS

-   BENDER

-   ALL

-   FORWARD

-   BACK

-   SLOW DOWN

-   FASTER

-   RIGHT

-   LEFT

-   STOP

-   QUIT

To control the robots make sure to do the following:

-   To control robot 1:

    -   Start your utterance with JARVIS followed by the command(s) you
        > want

-   To control robot 2:

    -   Start your utterance with BENDER followed by the command(s) you
        > want

-   To control both robots together:

    -   Start your utterance with ALL followed by the command(s) you
        > want

Take into consideration that some commands take precedence over others:

-   FORWARD &gt;&gt; BACK &gt;&gt;

-   RIGHT &gt;&gt; LEFT

-   SLOW DOWN &gt;&gt; FASTER

-   QUIT &gt;&gt; other commands

Possible future improvements:

-   Add a GUI

-   Make the number of robots dynamic and chosen on run time

-   Grow the corpus of words, possibly introduce a wander function
    > and/or more complex functions

-   Use the sonar to override commands that may cause the robot to go
    > into unfavorable situations.
