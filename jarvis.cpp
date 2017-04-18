#include <iostream>
#include <string>
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <thread>
#include "threadsafequeue.h"

using namespace std;

void recognize_from_microphone(ThreadSafeQueue<string>& q);
void consume(ThreadSafeQueue<string>& q);

ps_decoder_t *ps;                  // create pocketsphinx decoder structure
cmd_ln_t *config;                  // create configuration structure
ad_rec_t *ad;                      // create audio recording structure - for use with ALSA functions

int16 adbuf[4096];                 // buffer array to hold audio data
uint8 utt_started, in_speech;      // flags for tracking active speech - has speech started? - is speech currently happening?
int32 k;                           // holds the number of frames in the audio buffer
int32 score = 80;					// holds the score associated with the hypothesis. This sets the threshold 
char const *hyp;                   // pointer to "hypothesis"

void *mic_read()
{
    ad_start_rec(ad);
    ps_start_utt(ps);
}


int main(int argc, char *argv[])
{

  config = cmd_ln_init(NULL, ps_args(), TRUE,                   // Load the configuration structure - ps_args() passes the default values
    "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us",  // path to the standard english language model
    "-lm", "custom.lm",                                         // custom language model (file must be present)
    "-dict", "custom.dic",                                      // custom dictionary (file must be present)
    "-logfn", "/dev/null",                                      // suppress log info
     NULL);

  ps = ps_init(config);                                                        // initialize the pocketsphinx decoder
  ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"), (int) cmd_ln_float32_r(config, "-samprate")); // open default microphone at default samplerate
  
  ThreadSafeQueue<string> q;									//instantiate the thread safe queue
  
  using namespace std::placeholders;
  
  
  //command producer thread
  std::thread producer(std::bind(&recognize_from_microphone, std::ref(q)));
  
  //command consumer thread
  std::thread consumer(std::bind(&consume, std::ref(q)));
  
  producer.join();
  consumer.join();

 ad_close(ad);                                                    // close the microphone
}

void consume(ThreadSafeQueue<string>& q)
{
	while(1)
	{
		string command = q.pop();
		cout << "Decoded Command Sequence: "<< command << "\n" <<endl; 
	} 
}

void recognize_from_microphone(ThreadSafeQueue<string>& q)
{

    ad_start_rec(ad);                                // start recording
    ps_start_utt(ps);                                // mark the start of the utterance
    utt_started = FALSE;                             // clear the utt_started flag

    while(1) {
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
            q.push(hyp);							 //push the hypothesis into the queue
            
            //return hyp;                              // the function returns the hypothesis
            //break;                                   // exit the while loop and return to main
        }
    }
}
