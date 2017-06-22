# jetnet
A wrapper around the ancient JETNET 3.4 Fortran-based neural network package.

## Setup
```
	source setup.sh
```

## Build
```
    make
```

## Test
```
    cd example
    ./train.py
```
Note: In this example, the training is deliberately driven well beyond
where it overfits. This is shown in the time series plot, which shows
RMS(test),  RMS(train) vs. training epoch (cycle). In general, the error rate on the test sample exceeds that on the training sample. However, we expect the gap between the two RMS values to grow without limit in the overfitting regime.

The output of the training is the C++ function
```
    ttbarnet.cpp
```
which can be called as follows using PyROOT
```
    gROOT.ProcessLine('.L ttbarnet.cpp')
       :    :
    D = ttbarnet(....)
	```
	
