#include "MicroBit.h"
#include "SerialStreamer.h"
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#include "StreamNormalizer.h"
#include "StreamSplitter.h"
#include "MicroBitAudioProcessor.h"
#include "Tests.h"


static NRF52ADCChannel *mic = NULL;
static SerialStreamer *streamer = NULL;
static StreamNormalizer *processor = NULL;
static LevelDetector *level = NULL;
static LevelDetectorSPL *levelSPL = NULL;
static StreamSplitter *splitter = NULL;
static MicroBitAudioProcessor *fft = NULL;
static int claps = 0;
static volatile int sample;

void
onLoud(MicroBitEvent)
{
    DMESG("LOUD");
    claps++;
    if (claps >= 10)
        claps = 0;

    uBit.display.print(claps);
}

void
onQuiet(MicroBitEvent)
{
    DMESG("QUIET");
}

void mems_mic_drift_test()
{
    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    while(true)
    {
        sample = uBit.io.P0.getAnalogValue();
        uBit.sleep(250);

        sample = uBit.io.microphone.getAnalogValue();
        uBit.sleep(250);

        uBit.display.scroll(sample);
    }
}

void
mems_mic_test()
{
    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);          // Uncomment for v1.47.2
        //mic->setGain(7,1);        // Uncomment for v1.46.2
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 0.05f, true, DATASTREAM_FORMAT_8BIT_SIGNED);

    if (streamer == NULL)
        streamer = new SerialStreamer(processor->output, SERIAL_STREAM_MODE_BINARY);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    while(1)
        uBit.sleep(1000);
}

void
mems_clap_test(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (splitter == NULL)
        splitter = new StreamSplitter(processor->output);

    if (level == NULL)
        level = new LevelDetector(*splitter, 150, 75);

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3)){
        uBit.sleep(1000);
    }

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

void
mems_clap_test_2(int wait_for_clap)
{
    claps = 0;

    // if (mic == NULL){
    //     mic = uBit.adc.getChannel(uBit.io.microphone);
    //     mic->setGain(7,0);
    // }

    // if (processor == NULL)
    //     processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    // if (splitter == NULL)
    //     splitter = new StreamSplitter(processor->output);

    // if (level == NULL)
    //     level = new LevelDetector(*splitter, 150, 75);

    // uBit.io.runmic.setDigitalValue(1);
    // uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3)){
        uBit.sleep(1000);
    }

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

void
mems_clap_test_spl(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (levelSPL == NULL)
        levelSPL = new LevelDetectorSPL(processor->output, 75.0, 60.0, 9, 52, DEVICE_ID_MICROPHONE);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);
}

class MakeCodeMicrophoneTemplate {
  public:
    MIC_DEVICE microphone;
    LevelDetectorSPL level;
    MakeCodeMicrophoneTemplate() MIC_INIT { MIC_ENABLE; }
};

void
mc_clap_test()
{
    new MakeCodeMicrophoneTemplate();

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(1)
    {
        uBit.sleep(1000);
    }
}
