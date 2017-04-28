
#include "threadsafequeue.h"
#include <iostream>
#include <string>
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <ps_search.h>
#include "Aria.h"
#include <stdlib.h>
#include <stdio.h>



using namespace std;

void producecommand(ThreadSafeQueue<string>& q, ThreadSafeQueue<string>& q2);
void consume(ThreadSafeQueue<string>& q, ArRobot& robot);

ps_decoder_t *ps;                  // create pocketsphinx decoder structure
cmd_ln_t *config;                  // create configuration structure
ad_rec_t *ad;                      // create audio recording structure - for use with ALSA functions

int16 adbuf[4096];                 // buffer array to hold audio data
uint8 utt_started, in_speech;      // flags for tracking active speech - has speech started? - is speech currently happening?
int32 k;                           // holds the number of frames in the audio buffer
int32 score;					// holds the score associated with the hypothesis. This sets the threshold
std::mutex mutex_;
ArRobot robot;
ArRobot robot2;
bool global_running;

const char* host1 = "10.0.126.14";
const char* host2 = "10.0.126.15";
int port = 8101;
const string rname2 = "BENDER";
const string rname = "JARVIS";
const string ALL = "ALL";
char const *hyp;                   // pointer to "hypothesis"


int main(int argc, char *argv[])
{
  Aria::init();

  ArTcpConnection con1;
  ArSonarDevice sonar1;

  ArTcpConnection con2;
  ArSonarDevice sonar2;

  int ret;
  std::string str;
  ArLog::log(ArLog::Normal, "Connecting to first robot at %s:%d...", host1, port);
  if ((ret = con1.open(host1, port)) != 0)
  {
    str = con1.getOpenMessage(ret);
    printf("Open failed to robot 1: %s\n", str.c_str());
    Aria::exit(1);
    return 1;
  }

  robot.addRangeDevice(&sonar1);
  robot.setDeviceConnection(&con1);

  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot 1... exiting\n");
    Aria::exit(1);
    return 1;
  }

  ArLog::log(ArLog::Normal, "Connecting to first robot at %s:%d...", host2, port);
  if ((ret = con2.open(host2, port)) != 0)
  {
    str = con2.getOpenMessage(ret);
    printf("Open failed to robot 1: %s\n", str.c_str());
    Aria::exit(1);
    return 1;
  }

  robot2.addRangeDevice(&sonar2);
  robot2.setDeviceConnection(&con2);

  if (!robot2.blockingConnect())
  {
    printf("Could not connect to robot 1... exiting\n");
    Aria::exit(1);
    return 1;
  }

  robot.runAsync(true);
  robot2.runAsync(true);


  config = cmd_ln_init(NULL, ps_args(), TRUE,                   // Load the configuration structure - ps_args() passes the default values
  "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us",  // path to the standard english language model
  "-lm", "custom.lm",                                         // custom language model (file must be present)
  "-dict", "custom.dic",                                      // custom dictionary (file must be present)
  "-logfn", "/dev/null",                                      // suppress log info
  NULL);

  ps = ps_init(config);                                                        // initialize the pocketsphinx decoder
  ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"), (int) cmd_ln_float32_r(config, "-samprate")); // open default microphone at default samplerate

  ThreadSafeQueue<string> q;									//instantiate the thread safe queue
  ThreadSafeQueue<string> q2;

  using namespace std::placeholders;

  system("canberra-gtk-play -f jarvissounds/jbl_begin.ogg");
  global_running = true;

  //command producer thread
  std::thread producer(std::bind(&producecommand, std::ref(q), std::ref(q2)));

  //command consumer thread
  std::thread consumer(std::bind(&consume, std::ref(q), std::ref(robot)));
  std::thread consumer2(std::bind(&consume, std::ref(q2), std::ref(robot2)));

  producer.join();
  consumer.join();
  consumer2.join();

  robot.waitForRunExit();
  robot2.waitForRunExit();
  Aria::exit(0);
  ad_close(ad);                                                    // close the microphone
  return 0;
}

void consume(ThreadSafeQueue<string>& q, ArRobot& robot)
{
  while(global_running)
  {
    auto command = q.pop();
    cout << command << endl;
    std::unique_lock<std::mutex> mlock(mutex_);
    if(command.find("FORWARD") != std::string::npos)
    {
      cout << "Moving Forward...." << endl;
      robot.setRotVel(0);
      robot.setVel(150);
    }
    else if(command.find("BACK") != std::string::npos)
    {
      cout << "Backing Up...." << endl;
      robot.setRotVel(0);
      robot.setVel(-150);
    }
    else if(command.find("STOP") != std::string::npos)
    {
      cout << "Stopping...." << endl;
      robot.setRotVel(0);
      robot.setVel(0);
    }

    if(command.find("RIGHT") != std::string::npos)
    {
      cout << "Turning Right...." << endl;
      robot.setRotVel(-30);
    }
    else if(command.find("LEFT") != std::string::npos)
    {
      cout << "Turning Left...." << endl;
      robot.setRotVel(30);
    }

    if(command.find("SLOW DOWN") != std::string::npos)
    {
      cout << "Slowing Down...." << endl;
      if(robot.getVel()<900 && robot.getVel() > 0)
      {
        robot.setVel((robot.getVel()-100));
      }
      else if(robot.getVel()>-900 && robot.getVel() < 0)
      {
        robot.setVel((robot.getVel()+100));
      }
      else
      {
        std::cout << "Already at full stop" << endl;
      }
    }
    else if(command.find("FASTER") != std::string::npos)
    {
      cout << "Gotta Go Fast...." << endl;
      if(robot.getVel()<900 && robot.getVel()>=0)
      {
        robot.setVel((robot.getVel()+100));
      }
      else if (robot.getVel()>-900 && robot.getVel()<0)
      {
        robot.setVel(robot.getVel()-100);
      }
      else
      {
        std::cout << "Too Fast" << endl;
      }

    }
    if(command.find("WANDER") != std::string::npos)
    {
      robot.stop();
      robot.clearDirectMotion();
      cout<< "Now in Wander mode" << endl;

      ArActionStallRecover recover;
      ArActionBumpers bumpers;
      ArActionAvoidFront avoidFrontNear("Avoid Front Near", 225, 0);
      ArActionAvoidFront avoidFrontFar;
      ArActionConstantVelocity constantVelocity("Constant Velocity", 400);

      robot.addAction(&recover, 100);
      robot.addAction(&bumpers, 75);
      robot.addAction(&avoidFrontNear, 50);
      robot.addAction(&avoidFrontFar, 49);
      robot.addAction(&constantVelocity, 25);
      usleep(1000);

    }
    if(command.find("QUIT") != std::string::npos)
    {
      robot.stop();
      global_running == false;
      std::exit(0);
    }
    robot.enableMotors();
    robot.enableSonar();
    robot.comInt(ArCommands::ENABLE,1);
    mlock.unlock();
    usleep(200);
  }

}


void producecommand(ThreadSafeQueue<string>& q, ThreadSafeQueue<string>& q2)
{
  ad_start_rec(ad);                                // start recording
  ps_start_utt(ps);                                // mark the start of the utterance
  utt_started = FALSE;                             // clear the utt_started flag
  while(global_running) {

    k = ad_read(ad, adbuf, 4096);                // capture the number of frames in the audio buffer
    ps_process_raw(ps, adbuf, k, FALSE, FALSE);  // send the audio buffer to the pocketsphinx decoder

    in_speech = ps_get_in_speech(ps);            // test to see if speech is being detected

    if (in_speech && !utt_started) {             // if speech has started and utt_started flag is false
      utt_started = TRUE;                      // then set the flag
    }

    if (!in_speech && utt_started) {             // if speech has ended and the utt_started flag is true
      ps_end_utt(ps);                          // then mark the end of the utterance
      ad_stop_rec(ad);                         // stop recording
      hyp = ps_get_hyp(ps, &score );             // query pocketsphinx for "hypothesis" of decoded statement
      //cout << "The hypothesis score is: " << score << endl;

      string speech = hyp;
      if(speech.compare(0,rname.length(),rname)== 0)
      {
        q.push(speech);							 //push the hypothesis into the queue
        cout << "Command Recognized. Jarvis Running...." << endl;
      }
      else if (speech.compare(0,rname2.length(),rname2)==0)
      {
        q2.push(speech);
        cout << "Command Recognized. Bender Running...." << endl;
      }
      else if (speech.compare(0,ALL.length(),ALL)==0)
      {
        q.push(speech);
        q2.push(speech);
        cout << "This will be a doozy!" << endl;
      }
      else
      {
        cout << rname <<" and "<< rname2 <<" ignore the command" << endl;
      }


      ad_start_rec(ad);                                // start recording
      ps_start_utt(ps);                                // mark the start of the utterance
      utt_started = FALSE;                             // clear the utt_started flag
    }
  }
  std::exit(0);
}
