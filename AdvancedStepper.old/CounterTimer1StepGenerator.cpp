// 
// 
// 

#include "CounterTimer1StepGenerator.h"
#include "IStepSequencer.h"

CounterTimer1StepGenerator::CounterTimer1StepGenerator()
			{
			Initialize();
			}

void CounterTimer1StepGenerator::Stop()
	{
	Initialize();
	}

void CounterTimer1StepGenerator::Start(float stepsPerSecond, IStepSequencer *sequencer)
	{
	activeSequencer = sequencer;
	stepState = false;
	SetStepRate(stepsPerSecond);
	OCR1A = nextCompareValue;
	TCNT1 = 0;
	TCCR1A = 0x00;				// Generate no outputs 
	TCCR1B = 0x09;				// No prescale, CTC mode, compare with OCR1A
	BitSet(TIMSK1, OCIE1A);		// Enable OCR1A interrupt
	}

void CounterTimer1StepGenerator::SetStepRate(float stepsPerSecond)
	{
	uint16_t counts = ComputeCountsFromStepsPerSecond(stepsPerSecond);
	//OCR1A = counts;
	nextCompareValue = counts;
	}

void CounterTimer1StepGenerator::TimerCompareInterruptService()
	{
	stepState = !stepState;
	activeSequencer->Step(stepState); // Instruct the active sequencer to make a step
	OCR1A = nextCompareValue;	// load the next compare value, which could have been recomputed externally.
	}

void CounterTimer1StepGenerator::Initialize()
	{
	TCCR1B = 0x00;				// Stop Timer Counter 1 
	TCNT1 = 0;					// Clear Timer Counter 1 
	BitClear(TIMSK1, OCIE1A);	// Disable OCR1A interrupts
	OCR1A = 0xFF;
	OCR1B = 0xFF;
	TCCR1A = 0x00;				// Generate no outputs 
	stepState = false;			// Ensure that we start on a rising edge.
	}

uint16_t CounterTimer1StepGenerator::ComputeCountsFromStepsPerSecond(float stepsPerSecond)
	{
	// Two interrupts are required for each step
	auto frequency = stepsPerSecond * 2.0;
	if (frequency > Fmax) frequency = Fmax;
	if (frequency < Fmin) frequency = Fmin;
	uint16_t counts = Fmax / frequency;
	return counts;
	}

IStepSequencer *CounterTimer1StepGenerator::activeSequencer = NULL;
uint16_t CounterTimer1StepGenerator::nextCompareValue = 65535;
bool CounterTimer1StepGenerator::stepState = false;


ISR(TIMER1_COMPA_vect)
	{
	CounterTimer1StepGenerator::TimerCompareInterruptService();
	}
