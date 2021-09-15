import os
import torch
import threading

import numpy as np

from skimage import io
from torch.utils.data import Dataset
from torchvision import transforms
from progress.bar import IncrementalBar


# column_names:
#['Ps_dBm', 'RSRP', 'altitude', 'bandwidth', 'cell_altitude',
#    'cell_height', 'cell_lat', 'cell_lon', 'dist3d', 'frequency', 'height',
#    'indoorDist', 'indoorIntersections', 'lat', 'lat_dist', 'lon',
#    'lon_dist', 'offset', 'terrainDist', 'terrainIntersections', 'id',
#    'area']

class MyDataset(Dataset):
    def __init__(self, base_path, features, feature_scaler, target_scaler, image_size, image_range, columns, image_model, data_augmentation, load_images=True, sv=False, tv=False, num_workers=0, test=False, REM=False, mapped_heights=True, corrected_position=True, path=None): # scalers?
        self.base_path = base_path

        self.test = test
        self.REM = REM

        self.folder_ending = ''
        if not mapped_heights:
            self.folder_ending += '_noHeights'
        if not corrected_position:
            self.folder_ending += '_noPositionCorrection'
        
        # save features and target values
        self.columns = columns
        self.features = features[self.columns].to_numpy(dtype=np.float32)
        if not self.REM:
            self.targets = features["y"].to_numpy(dtype=np.float32)

        # save model parameters
        self.image_model = image_model
        self.sv = sv
        self.tv = tv
        self.data_augmentation = data_augmentation

        # save other values
        if self.test:
            self.RSRP = features["RSRP"].to_numpy(dtype=np.float32)
            self.ref = features["ref"].to_numpy(dtype=np.float32)
            self.offset = features["offset"].to_numpy(dtype=np.float32)
            self.dist2d = features["dist2d"].to_numpy(dtype=np.float32)
        if self.test or self.REM:
            self.pathloss = features["pathloss"].to_numpy(dtype=np.float32)

        # save scalers
        self.feature_scaler = feature_scaler
        self.target_scaler = target_scaler

        # save image information
        if not self.REM:
            self.city = features["city"].to_numpy()
            self.image_folder = features["area"].to_numpy()
        else:
            self.path = path
        self.height = features["height"].to_numpy()
        self.sample_id = features["id"].to_numpy()
        self.image_size = image_size
        self.image_range = image_range.copy()
        
        for i in range(len(image_range)):
            if image_range[i] != -1:
                self.image_range[i] = str(image_range[i]) + 'm'
            else:
                self.image_range[i] = 'full'


        self.image_path = self.base_path + 'dataset_processed'
        
        # save number of data samples
        self.length = len(features)

        # load images in advance
        self.load_images = load_images
        if self.data_augmentation > 0:
            self.load_images = False
            
        if self.image_model and self.load_images:
            if self.sv and self.tv:
                self.images = torch.zeros(len(self.sample_id),128,64)
            else:
                self.images = torch.zeros(len(self.sample_id),64,64)

            if num_workers == 0:
                bar = IncrementalBar('read images', max = len(self.sample_id))
                for i in range(len(self.sample_id)):
                    self.images[i] = self.load_image(i)[0]
                    bar.next()
            else:
                threads = list()
                for i in range(num_workers):
                    x = threading.Thread(target=self.thread_function, args = (num_workers, i))
                    threads.append(x)
                    x.start()

                for thread in threads:
                    thread.join()



    def __len__(self):
        return self.length



    def __getitem__(self, index):
        if self.image_model:
            if self.load_images:
                A = self.images[index].unsqueeze_(0)
            else:
                A = self.load_image(index)
        else:
            A = 0

        # get features
        X_norm = self.feature_scaler.transform(self.features[index].reshape(1, -1))
        X = torch.from_numpy(X_norm).float().squeeze()
        X = X.type(torch.float32)

        # get target
        if not self.REM:
            y_norm = self.target_scaler.transform(self.targets[index].reshape(1, -1))
            y = torch.from_numpy(y_norm).float().squeeze(1)

        if self.REM:
            return X, A, self.pathloss[index], self.sample_id[index]
        elif self.test:
            return X, A, y, self.RSRP[index], self.pathloss[index], self.ref[index], self.offset[index], self.dist2d[index], self.sample_id[index], self.features[index][1], self.image_folder[index], self.height[index], self.city[index]
        else:
            return X, A, y#, time_im, time_f



    def load_image(self, index):
        sample = self.sample_id[index]
        if self.REM:
            image_path_sv = self.path + 'processed/sv/' + self.image_range[0] + '/' + str(sample).zfill(4) + '.png'
            image_path_tv = self.path + 'processed/tv/' + self.image_range[1] + '/' + str(sample).zfill(4) + '.png'
        else:
            area = self.image_folder[index]
            if self.city[index] == 'aarhus':
                if self.height[index] == 1.5:
                    area += '_DT'
                else:
                    area += '_' + str(int(self.height[index])) + 'm'

            if self.city[index] == 'dortmund':
                image_path_sv = self.image_path + self.folder_ending + '/' + self.city[index] + '/' + str(self.image_size) + '/sv/' + self.image_range[0] + '/' + area + '/' + str(sample) + '.png'
                image_path_tv = self.image_path + self.folder_ending + '/' + self.city[index] + '/' + str(self.image_size) + '/tv/' + self.image_range[1] + '/' + area + '/' + str(sample) + '.png'
            else:
                image_path_sv = self.image_path + '/' + self.city[index] + '/' + str(self.image_size) + '/sv/' + self.image_range[0] + '/' + area + '/' + str(sample) + '.png'
                image_path_tv = self.image_path + '/' + self.city[index] + '/' + str(self.image_size) + '/tv/' + self.image_range[1] + '/' + area + '/' + str(sample) + '.png'
            
        if self.data_augmentation > 0:
            composed = transforms.Compose([transforms.ToPILImage(),  transforms.Grayscale(), transforms.RandomAffine(self.data_augmentation, shear=10), transforms.ToTensor()])
        else:
            composed = transforms.Compose([transforms.ToPILImage(),  transforms.Grayscale(), transforms.ToTensor()])
        
        # read side view image
        if self.sv:
            try:
                image = io.imread(image_path_sv)
            except:
                print("image not found")
            image = image / 255
            sv = torch.from_numpy(image).float().permute(2,0,1) # switch axis
            sv = composed(sv)
            A = sv

        # read top view image
        if self.tv:
            try:
                image = io.imread(image_path_tv)
            except:
                print("image not found")
            image = image / 255
            tv = torch.from_numpy(image).float().permute(2,0,1) # switch axis
            tv = composed(tv)

            if self.sv:
                A = torch.Tensor(torch.cat((sv[0],tv[0]),0)).unsqueeze_(0)
            else:
                A = tv

        return A


    def thread_function(self, num_workers, num):
        for i in range(len(self.sample_id)):
            if i % num_workers == num:
                self.images[i] = self.load_image(i)[0]