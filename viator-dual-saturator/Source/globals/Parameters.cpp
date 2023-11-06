#include "Parameters.h"


ViatorParameters::Params::Params()
{
    initSliderParams();
    initButtonParams();
    initMenuParams();
}

void ViatorParameters::Params::initSliderParams()
{
    using skew = SliderParameterData::SkewType;
    using type = SliderParameterData::NumericType;
    
    _sliderParams.push_back({ViatorParameters::gainID, ViatorParameters::gainName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::evenDriveID, ViatorParameters::evenDriveName, 0.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::evenID, ViatorParameters::evenName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::oddDriveID, ViatorParameters::oddDriveName, 0.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::oddID, ViatorParameters::oddName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::volumeID, ViatorParameters::volumeName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::mixID, ViatorParameters::mixName, 0.0f, 100.0f, 100.0f, skew::kNoSkew, 0.0, type::kInt});
    _sliderParams.push_back({ViatorParameters::cutoffID, ViatorParameters::cutoffName, 20.0f, 20000.0f, 0.0f, skew::kSkew, 1000.0, type::kInt});
}

void ViatorParameters::Params::initButtonParams()
{
    _buttonParams.push_back({ViatorParameters::evenBypassID, ViatorParameters::evenBypassName, false});
    _buttonParams.push_back({ViatorParameters::oddBypassID, ViatorParameters::oddBypassName, false});
}

void ViatorParameters::Params::initMenuParams()
{
}
