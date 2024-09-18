#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class BackgroundComponent : public Component
{
public:
    BackgroundComponent();
    void paint(Graphics &g) override;

private:
    Image backgroundImage;
};

//==============================================================================

class KnobLookAndFeel : public LookAndFeel_V4
{
public:
    KnobLookAndFeel(Image knobImage);

    void drawRotarySlider(Graphics &g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          Slider &slider) override;

private:
    Image knobImage;
};

//==============================================================================

class VibratoAudioProcessorEditor : public AudioProcessorEditor, private Timer
{
public:
    VibratoAudioProcessorEditor (VibratoAudioProcessor&);
    ~VibratoAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    VibratoAudioProcessor& processor;

    BackgroundComponent* backgroundComponent;
    OwnedArray<Slider> knobs;

    OwnedArray<Label> labels;
    Array<Component*> components;

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    OwnedArray<SliderAttachment> knobAttachments;

    void timerCallback() override;
    void updateUIcomponents();
    Label pitchShiftLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VibratoAudioProcessorEditor)
};
