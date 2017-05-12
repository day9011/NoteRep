#ifndef MY_CONF
#define MY_CONF

//modified configure
#define sampFreq 8000 //in FrameExtractionOptions
#define frameLength 20 //in FrameExtractionOptions
#define lowFreq 20 //in MelBanksOptions
#define highFreq 3700 //in MelBanksOptions
#define numCeps 20 //in MfccOptions
#define vadEnergyThreshold 8 //in VadEnergyOptions
#define vadEnergyMeanScale 0.5 //in VadEnergyOptions
#define normVars (1==0) // in SlidingWindowCmnOptions
#define cmnCenter (1==1) // in SlidingWindowCmnOptions
#define cmnWindow 300 //in SlidingWindowCmnOptions
#define minPosterior 0.025 //in fgmm-global-gselect-to-post
#define PosteriorScale 1.0 //in fgmm-global-gselect-to-post
#define deltaWindow 3 //in DeltaFeaturesOptions and ShiftedDeltaFeaturesOptions
#define deltaOrder 2 // in DeltaFeaturesOptions
//default configure
//CudaMelBanksOptions
#define numBins 25
#define vtlnLow 100.0
#define vtlnHigh -500.0
#define vtlnWarp 1.0
#define debugMel (1==0)
#define htkMode (1==0)
//in my sre option
#define numIgnoreFrames 0
#define numGselect 20 //in gmm-select
#define Channel -1

//CudaFrameExtractionOptions

#endif
