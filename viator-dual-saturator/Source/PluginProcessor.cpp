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
    if (parameterID == "hqID")
    {
        if (newValue)
        {
            _spec.sampleRate = _evenHQModule->getOversamplingFactor() * getSampleRate();
            _evenBandFilter.prepare(_spec);
            _oddBandFilter.prepare(_spec);
        }
        
        else
        {
            _spec.sampleRate = getSampleRate();
            _evenBandFilter.prepare(_spec);
            _oddBandFilter.prepare(_spec);
        }
    }
}

void ViatordualsaturatorAudioProcessor::updateParameters()
{
}

//==============================================================================
void ViatordualsaturatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _evenHQModule = std::make_unique<juce::dsp::Oversampling<float>>(getTotalNumInputChannels(), 2, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    _evenHQModule->initProcessing(samplesPerBlock);
    _oddHQModule = std::make_unique<juce::dsp::Oversampling<float>>(getTotalNumInputChannels(), 2, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    _oddHQModule->initProcessing(samplesPerBlock);
    auto _hqState = _treeState.getRawParameterValue("hqID")->load();
    
    _spec.sampleRate = _hqState ? sampleRate * _evenHQModule->getOversamplingFactor() : sampleRate;
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    
    _evenBandFilter.prepare(_spec);
    _evenBandFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _evenDcFilter.prepare(_spec);
    _evenDcFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _oddBandFilter.prepare(_spec);
    _oddBandFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    _spec.sampleRate = sampleRate;
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
    const auto cutoff = _treeState.getRawParameterValue("cutoffID")->load();
    const auto _hqState = _treeState.getRawParameterValue("hqID")->load();

    _evenDcFilter.setCutoffFrequency(15.0);
    _inputGain.setGainDecibels(inputDB - 7.0);
    
    //--------------------------------------------------------------------------------
    // Define audio blocks for the buffers
    //--------------------------------------------------------------------------------
    juce::dsp::AudioBlock<float> block {buffer};
    juce::dsp::AudioBlock<float> hqBlock {buffer};
    
    _inputGain.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    if (_hqState)
    {
        hqBlock = _evenHQModule->processSamplesUp(block);
        evenOddProcess(juce::dsp::ProcessContextReplacing<float>(hqBlock));
        _evenHQModule->processSamplesDown(block);
    }
    
    else
    {
        evenOddProcess(juce::dsp::ProcessContextReplacing<float>(block));
    }
    
    
    //--------------------------------------------------------------------------------
    //Apply output volume
    //--------------------------------------------------------------------------------
    buffer.applyGain(outputGain);
}

template <typename ProcessContext>
void ViatordualsaturatorAudioProcessor::evenOddProcess(const ProcessContext& context) noexcept
{
    const auto oddGain = _treeState.getRawParameterValue("oddID")->load();
    const auto oddGainGain = juce::Decibels::decibelsToGain(oddGain);
    const auto evenGain = _treeState.getRawParameterValue("evenID")->load();
    const auto evenGainGain = juce::Decibels::decibelsToGain(evenGain);
    const auto oddDrive = _treeState.getRawParameterValue("oddDriveID")->load();
    const auto oddDriveGain = juce::Decibels::decibelsToGain(oddDrive);
    const auto evenDrive = _treeState.getRawParameterValue("evenDriveID")->load();
    const auto evenDriveGain = juce::Decibels::decibelsToGain(evenDrive);
    const auto oddBypass = _treeState.getRawParameterValue("oddBypassID")->load();
    const auto evenBypass = _treeState.getRawParameterValue("evenBypassID")->load();
    const auto mix = _treeState.getRawParameterValue("mixID")->load() * 0.01;
    _evenBandFilter.setCutoffFrequency(_treeState.getRawParameterValue("cutoffID")->load());
    _oddBandFilter.setCutoffFrequency(_treeState.getRawParameterValue("cutoffID")->load());
    
    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock      = context.getOutputBlock();
    const auto numChannels = outputBlock.getNumChannels();
    const auto numSamples  = outputBlock.getNumSamples();

    jassert (inputBlock.getNumChannels() == numChannels);
    jassert (inputBlock.getNumSamples()  == numSamples);

    if (context.isBypassed)
    {
        outputBlock.copyFrom (inputBlock);
        return;
    }
    
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* inputSamples  = inputBlock .getChannelPointer (channel);
        auto* outputSamples = outputBlock.getChannelPointer (channel);

        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            const auto xn = inputSamples[sample];
            const auto xnHigh = _evenBandFilter.processSample(static_cast<int>(channel), xn);
            
            // even
            auto xnEven = 0.65 * (xnHigh + (std::abs(xnHigh * evenDriveGain) - 0.5)) + xn;
            auto xnEvenOutput = xnEven * evenGainGain * evenBypass;
            
            // odd
            const auto xnOdd = (_piDivisor * std::atan(xnHigh * oddDriveGain) + xn) * 0.5;
            const auto xnOddOutput = xnOdd * oddGainGain * oddBypass;
            
            const auto xnWet = xnEvenOutput + xnOddOutput;
            const auto xnMix = (1.0 - mix) + xnWet + xn * mix;
            
            outputSamples[sample] = xnMix;
        }
    }
}

void ViatordualsaturatorAudioProcessor::applyEvenDistortion(juce::dsp::AudioBlock<float>& block, float drive, float cutoff)
{
    for (int channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto data = block.getChannelPointer(channel);
        for (int sample = 0; sample < block.getNumSamples(); ++sample)
        {
            auto xn = data[sample];
            auto xnHigh = _evenBandFilter.processSample(channel, xn);
            auto evenHarmonics = 0.65 * (xnHigh + (std::abs(xnHigh * drive) - 0.5));
            auto outputMix = evenHarmonics + xn;
            data[sample] = outputMix;
        }
    }
}

void ViatordualsaturatorAudioProcessor::applyOddDistortion(juce::dsp::AudioBlock<float>& block, float drive, float cutoff)
{
    for (int channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto data = block.getChannelPointer(channel);
        for (int sample = 0; sample < block.getNumSamples(); ++sample)
        {
            const auto xn = data[sample];
            const auto xnHigh = _oddBandFilter.processSample(channel, xn);
            const auto softClip = _piDivisor * std::atan(xnHigh * drive);
            const auto outputMix = softClip + xn;
            data[sample] = outputMix * 0.5;
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
