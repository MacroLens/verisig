#!/usr/bin/python

from tensorflow.keras import Input
from tensorflow.keras.layers import Dense
from tensorflow.keras.models import Sequential
import numpy as np
import sys
import yaml

ACTIVATION_MAP = {
    'Sigmoid': 'sigmoid',
    'Tanh': 'tanh',
    'Relu': 'relu',
    'Linear': 'linear',
}

def main(argv):
    input_filename = argv[0]
    output_filename = argv[1]

    with open(input_filename, 'r') as f:
        dnn_dict = yaml.safe_load(f)

    layer_counts = sorted(dnn_dict['weights'].keys())

    model = Sequential()
    for i, layer_count in enumerate(layer_counts):
        weights = np.array(dnn_dict['weights'][layer_count], dtype=np.float32).T
        offsets = np.array(dnn_dict['offsets'][layer_count], dtype=np.float32)
        activation = ACTIVATION_MAP[dnn_dict['activations'][layer_count]]

        input_dim, units = weights.shape
        if i == 0:
            model.add(Input(shape=(input_dim,)))
        model.add(Dense(units, activation=activation))
        model.layers[-1].set_weights([weights, offsets])

    model.save(output_filename)

if __name__ == '__main__':
    main(sys.argv[1:])
