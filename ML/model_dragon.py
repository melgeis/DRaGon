import torch
import numpy as np
from torch import nn



class ImageModel(nn.Module):
    # convBlock -> convBlock -> convBlock -> convBlock -> linear
    # convBlock: conv3d -> ReLU -> Batchnorm2d -> maxpool

    def __init__(self, args):
        super(ImageModel, self).__init__()

        self.blocks = nn.ModuleList()

        for i in range(len(args.out_channels)):
            if i == 0:
                block = ConvBlock(args.in_channels, args.out_channels[i], 0.2, kernel_size=args.kernels[i], max_pool=args.max_pool[i], padding=args.padding, stride=args.stride, dilation=args.dilation)
                if args.sv and args.tv:
                    output_size = block.output_size([args.image_size*2, args.image_size])
                else:
                    output_size = block.output_size([args.image_size, args.image_size])
            else:
                block = ConvBlock(args.out_channels[i-1], args.out_channels[i], 0.2, kernel_size=args.kernels[i], max_pool=args.max_pool[i], padding=args.padding, stride=args.stride, dilation=args.dilation)
                output_size = block.output_size(output_size)
            
            self.blocks.append(block)

        self.output_size = output_size[0] * output_size[1]
        self.output_layer = nn.Linear(self.output_size, self.output_size)

    def forward(self, x):
        batch_size = x.shape[0]
        for block in self.blocks:
            x = block(x)
        x = x.view(batch_size, -1)
        x = self.output_layer(x)
        return x


class FeatureModel(nn.Module):
    # Linear -> ReLU -> Batchnorm1d

    def __init__(self, num_features, out_channels, layers, p):
        super(FeatureModel, self).__init__()

        self.blocks = nn.ModuleList()
        
        for i in range(len(layers)):
            if i == 0:
                self.blocks.append(nn.Linear(num_features, layers[i]))
            else:
                self.blocks.append(nn.Linear(layers[i-1], layers[i]))
            self.blocks.append(nn.Dropout(p))
            self.blocks.append(nn.ReLU())
            self.blocks.append(nn.BatchNorm1d(layers[i]))
        
        self.output_layer = nn.Linear(layers[-1], out_channels)

    def forward(self, x):
        for block in self.blocks:
            x = block(x)

        x = self.output_layer(x)
        return x


class CombiModel(nn.Module):
    # FeatureModel + ImageModel -> NN
    # also Pathloss Model as input feature

    def __init__(self, args, target_scaler):
        super(CombiModel, self).__init__()

        self.cuda_enabled = args.cuda
        self.im_model = args.im_model
        self.feature_model = args.feature_model

        self.target_scaler = target_scaler

        # init ImageModel
        if self.im_model:
            self.ImageModel = ImageModel(args)
            self.im_model_out_size = self.ImageModel.output_size
        else:
            self.im_model_out_size = 100

        # init FeatureModel
        if self.feature_model:
            self.FeatureModel = FeatureModel(args.num_features, self.im_model_out_size, args.nn_layers, args.dropout)

        # combination of the models in neural network
        self.blocks = nn.ModuleList()
        self.blocks.append(nn.Linear(self.im_model_out_size, 16))
        self.blocks.append(nn.Dropout(args.dropout))
        self.blocks.append(nn.ReLU())
        self.blocks.append(nn.BatchNorm1d(16))
        self.blocks.append(nn.Linear(16,1))

        if self.cuda_enabled:
            try:
                self.ImageModel = self.ImageModel.cuda()
                print("image model cuda")
            except:
                pass
            try:
                self.FeatureModel = self.FeatureModel.cuda()
                print("feature model cuda")
            except:
                pass
            self = self.cuda()
        else:
            try:
                self.ImageModel = self.ImageModel.cpu()
                print("image model cpu")
            except:
                pass
            try:
                self.FeatureModel = self.FeatureModel.cpu()
                print("feature model cpu")
            except:
                pass
            self = self.cpu()

    def forward(self, image, features):
        x = 0
        if self.im_model:
            I = self.ImageModel(image)
            x += I
        if self.feature_model:
            F = self.FeatureModel(features)
            x += F

        for block in self.blocks:
            x = block(x)
        
        return x

class ConvBlock(nn.Module):
    def __init__(self, input_channels, output_channels, leaky_relu, **kwargs):
        super(ConvBlock, self).__init__()

        self.kernel_size = kwargs.get('kernel_size') if kwargs.get('kernel_size') else 1
        self.padding = kwargs.get('padding') if kwargs.get('padding') else 0
        self.stride = kwargs.get('stride') if kwargs.get('stride') else 1
        self.dilation = kwargs.get('dilation') if kwargs.get('dilation') else 1
        self.max_pool = kwargs.get('max_pool') if kwargs.get('max_pool') else 2

        self.conv = nn.Conv2d(input_channels, output_channels, kernel_size=self.kernel_size, padding=self.padding, stride=self.stride, dilation=self.dilation)
        self.dropout = nn.Dropout2d(kwargs.get('dropout') if kwargs.get('dropout') else 0)
        self.relu = nn.LeakyReLU(negative_slope=leaky_relu)
        self.batchnorm = nn.BatchNorm2d(output_channels)
        self.pool = nn.MaxPool2d(kernel_size=self.max_pool)

    def forward(self, x):
        x = self.conv(x)
        x = self.dropout(x)
        x = self.relu(x)
        x = self.batchnorm(x)
        x = self.pool(x)
        return x

    def output_size(self, h_w):
        h = np.floor( ((h_w[0] + (2 * self.padding) - (self.dilation * (self.kernel_size - 1) ) - 1 )/ self.stride) + 1)
        w = np.floor( ((h_w[1] + (2 * self.padding) - (self.dilation * (self.kernel_size - 1) ) - 1 )/ self.stride) + 1)
        size = [int(h/self.max_pool), int(w/self.max_pool)]
        return size
