# jetnet
A wrapper around the ancient JETNET 3.4 Fotran-based neural network package

BUILD
```
    make
```
TEST
```
    cd example
    ./train.py
    ./plot.py
```
Note: In this example, the training is deliberately driven well beyond where it overfits. This is shown in the time series plot, which shows RMS(test) - RMS(train) vs. training epoch (cycle). In general, the error rate on the test sample exceeds that on the training sample. However, we expect the gap between the two RMS values to grow without limit in the overfitting regime.
