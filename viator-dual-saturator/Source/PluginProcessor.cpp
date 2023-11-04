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
    juce::ScopedNoDenormals noDenormals;
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
