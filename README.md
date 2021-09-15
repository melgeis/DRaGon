DRaGon: Deep RAdio channel modeling from GeOinformatioN
========

This repository holds the source code of the novel machine learning-based DRaGon model, which can be used for automatic generation of Radio Environmental Maps
(REMs) from geographical data.

The data sample generation takes place in **LIMoSim**, which is a simulation framework for ground-based and aerial vehicular mobility, but is used here because of its environmental data aggregation capabilities.
The further processing of the generated data samples as well as the machine learning part are performed in *python* using *PyTorch*.

The repository also includes a model trained on all datasets as well as the required feature pickles for using this model on other data. 

- [**DATASET GENERATION INSTRUCTIONS**](DATASAMPLES.md)
- [**MACHINE LEARNING APPLICATION INSTRUCTIONS**](ML.md)


