# RealTimeVoiceControlAmiigoBot
Amiigo Voice control with a level of machine learning

Build the jarvis file using the following:
g++ -O3 -o jarvis jarvis.cpp  `pkg-config --cflags --libs pocketsphinx sphinxbase` -std=c++11

Build the jarvisRobot file using the following:
g++ -O3 -o jarvisRobot jarvisRobot.cpp  `pkg-config --cflags --libs pocketsphinx sphinxbase` -std=c++11 -fPIC -I/usr/local/Aria/include -L/usr/local/Aria/lib -lAria -lpthread -ldl -lrt
