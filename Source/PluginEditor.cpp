#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// BackgroundComponent Implementation

BackgroundComponent::BackgroundComponent()
{
    // Load the background image from binary resources
    backgroundImage = ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
}

void BackgroundComponent::paint(Graphics &g)
{
    // Set the background to 200x359 size
    Rectangle<float> bounds(0, 0, 200, 359);
    g.drawImage(backgroundImage, bounds);
}

//==============================================================================
// KnobLookAndFeel Implementation

KnobLookAndFeel::KnobLookAndFeel(Image knobImage) : knobImage(knobImage) {}

void KnobLookAndFeel::drawRotarySlider(Graphics &g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                       Slider &slider)
{
    const int numFrames = knobImage.getHeight() / knobImage.getWidth();
    const int frameIndex = static_cast<int>(sliderPos * (numFrames - 1));
    const int frameHeight = knobImage.getWidth();

    const int knobWidth = 64;
    const int knobHeight = 64;
    const int centerX = x + (width - knobWidth) / 2;
    const int centerY = y + (height - knobHeight) / 2;

    g.drawImage(knobImage, centerX, centerY, knobWidth, knobHeight,
                0, frameIndex * frameHeight,
                knobWidth, frameHeight);
}

//==============================================================================
// VibratoAudioProcessorEditor Implementation

VibratoAudioProcessorEditor::VibratoAudioProcessorEditor (VibratoAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible(backgroundComponent = new BackgroundComponent());

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();

    for (int i = 0; i < parameters.size(); ++i) {
        if (const AudioProcessorParameterWithID* parameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {
            if (processor.parameters.parameterTypes[i] == "Slider") {
                // Create knob (replacing the Slider)
                Slider* knob;
                knobs.add(knob = new Slider(Slider::Rotary, Slider::NoTextBox));

                Image knobImage = ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
                knob->setLookAndFeel(new KnobLookAndFeel(knobImage));

                SliderAttachment* knobAttachment;
                knobAttachments.add(knobAttachment = new SliderAttachment(processor.parameters.apvts, parameter->paramID, *knob));

                components.add(knob);
            }

            components.getLast()->setName(parameter->name);
            components.getLast()->setComponentID(parameter->paramID);
            addAndMakeVisible(components.getLast());
        }
    }

    addAndMakeVisible(&pitchShiftLabel);
    setSize(200, 379);
    startTimer(50);
}

VibratoAudioProcessorEditor::~VibratoAudioProcessorEditor()
{
    for (auto* knob : knobs)
        knob->setLookAndFeel(nullptr); // Reset LookAndFeel to avoid dangling pointer issues
}

void VibratoAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void VibratoAudioProcessorEditor::resized()
{
    backgroundComponent->setBounds(getLocalBounds());
    
    Rectangle<int> area = getLocalBounds().reduced(10);
    
    // Misalkan posisi dan ukuran knob disimpan dalam array atau vektor
int knobWidths[] = {64, 64, 64}; // Lebar masing-masing knob
int knobHeights[] = {64, 64, 64}; // Tinggi masing-masing knob
int knobXPositions[] = {20, 110, 150}; // Posisi X masing-masing knob
int knobYPositions[] = {40, 40, 10}; // Posisi Y masing-masing knob

for (int i = 0; i < components.size(); ++i) {
    if (Slider* knob = dynamic_cast<Slider*>(components[i])) {
        // Atur posisi dan ukuran knob secara manual
        int width = (i < sizeof(knobWidths)/sizeof(knobWidths[0])) ? knobWidths[i] : 64; // Gunakan lebar default jika tidak ada data
        int height = (i < sizeof(knobHeights)/sizeof(knobHeights[0])) ? knobHeights[i] : 64; // Gunakan tinggi default jika tidak ada data
        int x = (i < sizeof(knobXPositions)/sizeof(knobXPositions[0])) ? knobXPositions[i] : 10; // Gunakan posisi X default jika tidak ada data
        int y = (i < sizeof(knobYPositions)/sizeof(knobYPositions[0])) ? knobYPositions[i] : 10; // Gunakan posisi Y default jika tidak ada data

        knob->setBounds(x, y, width, height);
    }
}

 /*   for (int i = 0; i < components.size(); ++i) {
        if (Slider* knob = dynamic_cast<Slider*>(components[i])) {
            knob->setBounds(area.removeFromTop(64).reduced(10));
        }
    }*/
}

void VibratoAudioProcessorEditor::timerCallback()
{
    updateUIcomponents();
}

void VibratoAudioProcessorEditor::updateUIcomponents()
{
    float minPitch = 0.0f;
    float maxPitch = 0.0f;
    float minSpeed = 1.0f;
    float maxSpeed = 1.0f;
    String pitchShiftText = "";

    float width = processor.paramWidth.getTargetValue();
    float frequency = processor.paramFrequency.getTargetValue();
    int waveform = (int)processor.paramWaveform.getTargetValue();

    switch (waveform) {
        case VibratoAudioProcessor::waveformSine:
            minSpeed = 1.0f - M_PI * width * frequency;
            maxSpeed = 1.0f + M_PI * width * frequency;
            break;
        case VibratoAudioProcessor::waveformTriangle:
            minSpeed = 1.0f - 2.0f * width * frequency;
            maxSpeed = 1.0f + 2.0f * width * frequency;
            break;
        case VibratoAudioProcessor::waveformSawtooth:
            minSpeed = 1.0f - width * frequency;
            maxSpeed = 1.0f;
            break;
        case VibratoAudioProcessor::waveformInverseSawtooth:
            minSpeed = 1.0f;
            maxSpeed = 1.0f + width * frequency;
            break;
    }

    maxPitch = 12.0f * logf(maxSpeed) / logf(2.0f);

    if (minSpeed > 0.0f) {
        minPitch = 12.0f * logf(minSpeed) / logf(2.0f);
        pitchShiftText = String::formatted("Vibrato range: %+.2f to %+.2f semitones (speed %.3f to %.3f)",
                                           minPitch, maxPitch, minSpeed, maxSpeed);
    } else {
        pitchShiftText = String::formatted("Vibrato range: ----- to %+.2f semitones (speed %.3f to %.3f)",
                                           minPitch, maxPitch, minSpeed, maxSpeed);
    }

    pitchShiftLabel.setText(pitchShiftText, dontSendNotification);
}
