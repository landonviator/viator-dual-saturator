#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ViatordualsaturatorAudioProcessor::ViatordualsaturatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), _treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getButtonParams()[i].paramID, this);
    }
    
    // menus
    for (int i = 0; i < _parameterMap.getMenuParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getMenuParams()[i].paramID, this);
    }
}

ViatordualsaturatorAudioProcessor::~ViatordualsaturatorAudioProcessor()
{
    // sliders
     for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
     {
         _treeState.removeParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
     }
     
     // buttons
     for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
     {
         _treeState.removeParameterListener(_parameterMap.getButtonParams()[i].paramID, this);
     }
     
     // menus
     for (int i = 0; i < _parameterMap.getMenuParams().size(); i++)
     {
         _treeState.removeParameterListener(_parameterMap.getMenuParams()[i].paramID, this);
     }
}

//==============================================================================
const juce::String ViatordualsaturatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ViatordualsaturatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ViatordualsaturatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ViatordualsaturatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ViatordualsaturatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ViatordualsaturatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ViatordualsaturatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ViatordualsaturatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ViatordualsaturatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void ViatordualsaturatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ViatordualsaturatorAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        auto param = _parameterMap.getSliderParams()[i];
        
        auto range = juce::NormalisableRange<float>(param.min, param.max);
        
        if (param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
        {
            range.setSkewForCentre(param.center);
        }

        if (param.isInt == ViatorParameters::SliderParameterData::NumericType::kInt)
        {
            params.push_back (std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(juce::ParameterID { param.paramID, _versionNumber }, param.paramName, param.paramName, range, param.initial, valueToTextFunction, textToValueFunction));
        }
        
        else
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { param.paramID, _versionNumber }, param.paramName, range, param.initial));
        }
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        auto param = _parameterMap.getButtonParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { param.paramID, _versionNumber }, param.paramName, _parameterMap.getButtonParams()[i].initial));
    }
    
    // menus
    for (int i = 0; i < _parameterMap.getMenuParams().size(); i++)
    {
        auto param = _parameterMap.getMenuParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { param.paramID, _versionNumber }, param.paramName, param.choices, param.defaultIndex));
    }
    
    return { params.begin(), params.end() };
}

void ViatordualsaturatorAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
}

void ViatordualsaturatorAudioProcessor::updateParameters()
{
}

//==============================================================================
void ViatordualsaturatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.sampleRate = sampleRate;
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    
    _evenBandFilter.prepare(_spec);
    _evenBandFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _evenBandFilter.setCutoffFrequency(1000.0);
    _evenDcFilter.prepare(_spec);
    _evenDcFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _evenDcFilter.setCutoffFrequency(15.0);
    _oddBandFilter.prepare(_spec);
    _oddBandFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _oddBandFilter.setCutoffFrequency(1000.0);
    
    _oddGain.prepare(_spec);
    _oddGain.setRampDurationSeconds(0.02);
    _evenGain.prepare(_spec);
    _evenGain.setRampDurationSeconds(0.02);
    _inputGain.prepare(_spec);
    _inputGain.setRampDurationSeconds(0.02);
}

void ViatordualsaturatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ViatordualsaturatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ViatordualsaturatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //--------------------------------------------------------------------------------
    // Define local variables from parameters
    //--------------------------------------------------------------------------------
    const auto inputDB = _treeState.getRawParameterValue("gainID")->load();
    const auto outputDB = _treeState.getRawParameterValue("volumeID")->load();
    const auto outputGain = juce::Decibels::decibelsToGain(outputDB);
    const auto oddGain = _treeState.getRawParameterValue("oddID")->load();
    const auto evenGain = _treeState.getRawParameterValue("evenID")->load();
    const auto oddDrive = _treeState.getRawParameterValue("oddDriveID")->load();
    const auto oddDriveGain = juce::Decibels::decibelsToGain(oddDrive);
    const auto evenDrive = _treeState.getRawParameterValue("evenDriveID")->load();
    const auto evenDriveGain = juce::Decibels::decibelsToGain(evenDrive);
    const auto oddBypass = _treeState.getRawParameterValue("oddBypassID")->load();
    const auto evenBypass = _treeState.getRawParameterValue("evenBypassID")->load();
    const auto mix = _treeState.getRawParameterValue("mixID")->load() * 0.01;

    //--------------------------------------------------------------------------------
    // Define audio blocks for the buffers
    //--------------------------------------------------------------------------------
    juce::dsp::AudioBlock<float> oddBlock {_oddBuffer};
    juce::dsp::AudioBlock<float> evenBlock {_evenBuffer};
    juce::dsp::AudioBlock<float> block {buffer};
    
    //--------------------------------------------------------------------------------
    // Update gain modules for each buffer
    //--------------------------------------------------------------------------------
    _oddGain.setGainDecibels(oddGain < 0.0 ? oddGain * 4.0 : oddGain);
    _evenGain.setGainDecibels(evenGain < 0.0 ? evenGain * 4.0 : evenGain);
    _inputGain.setGainDecibels(inputDB - 7.0);
    _inputGain.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    //--------------------------------------------------------------------------------
    // Set odd and even buffers
    //--------------------------------------------------------------------------------
    _oddBuffer.makeCopyOf(buffer);
    _evenBuffer.makeCopyOf(buffer);
    
    //--------------------------------------------------------------------------------
    // Apply odd dsp
    //--------------------------------------------------------------------------------
    if (!oddBypass)
    {
        applyOddDistortion(_oddBuffer, oddDriveGain);
        _oddGain.process(juce::dsp::ProcessContextReplacing<float>(oddBlock));
    }
    
    //--------------------------------------------------------------------------------
    // Apply even dsp
    //--------------------------------------------------------------------------------
    if (!evenBypass)
    {
        applyEvenDistortion(_evenBuffer, evenDriveGain);
        _evenGain.process(juce::dsp::ProcessContextReplacing<float>(evenBlock));
    }
    
    //--------------------------------------------------------------------------------
    // Add odd signal to output
    //--------------------------------------------------------------------------------
    if (!oddBypass)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.addFrom(channel, 0, _oddBuffer, channel, 0, buffer.getNumSamples());
        }
    }
    
    //--------------------------------------------------------------------------------
    // Add even signal to output
    //--------------------------------------------------------------------------------
    if (!evenBypass)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.addFrom(channel, 0, _evenBuffer, channel, 0, buffer.getNumSamples());
        }
    }
    
    //--------------------------------------------------------------------------------
    // Apply output volume
    //--------------------------------------------------------------------------------
    buffer.applyGain(outputGain);
}

void ViatordualsaturatorAudioProcessor::applyEvenDistortion(juce::AudioBuffer<float> &buffer, float drive)
{
    auto data = buffer.getArrayOfWritePointers();
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto xn = data[channel][sample];
            auto xnHigh = _evenBandFilter.processSample(channel, xn);
            auto evenHarmonics = 0.65 * (xnHigh + (std::abs(xnHigh * drive) - 0.5));
            auto outputMix = _evenDcFilter.processSample(channel, evenHarmonics) * 0.5 + xn;
            data[channel][sample] = outputMix * 0.5;
        }
    }
}

void ViatordualsaturatorAudioProcessor::applyOddDistortion(juce::AudioBuffer<float> &buffer, float drive)
{
    auto data = buffer.getArrayOfWritePointers();
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto xn = data[channel][sample];
            const auto xnHigh = _oddBandFilter.processSample(channel, xn);
            const auto softClip = _piDivisor * std::atan(xnHigh * drive);
            const auto outputMix = softClip * 0.25 + xn;
            data[channel][sample] = outputMix * 0.5;
        }
    }
}

//==============================================================================
bool ViatordualsaturatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ViatordualsaturatorAudioProcessor::createEditor()
{
    //return new ViatordualsaturatorAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ViatordualsaturatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    _treeState.state.appendChild(variableTree, nullptr);
    juce::MemoryOutputStream stream(destData, false);
    _treeState.state.writeToStream (stream);
}

void ViatordualsaturatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    variableTree = tree.getChildWithName("Variables");
    
    if (tree.isValid())
    {
        _treeState.state = tree;
        _width = variableTree.getProperty("width");
        _height = variableTree.getProperty("height");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatordualsaturatorAudioProcessor();
}
