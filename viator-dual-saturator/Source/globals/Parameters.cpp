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
    
    _sliderParams.push_back({ViatorParameters::gainID, ViatorParameters::gainName, 0.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::evenID, ViatorParameters::evenName, -15.0f, 15.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::oddID, ViatorParameters::oddName, -15.0f, 15.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::volumeID, ViatorParameters::volumeName, -15.0f, 15.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
}

void ViatorParameters::Params::initButtonParams()
{
}

void ViatorParameters::Params::initMenuParams()
{
}
