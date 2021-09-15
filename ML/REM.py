import torch
import os

import pandas as pd

from experimentlogger import load_experiment
from easydict import EasyDict as edict
from tqdm import tqdm
from sklearn.metrics import mean_squared_error, mean_absolute_error

import init_model
import model_dragon
import process_data

from dataset import MyDataset
from helpers_ML import *


def predict(args, data_loader, model, target_scaler):
    bar = tqdm(total = len(data_loader))
    RSRP = dict()
    RSRP["DNN"] = []
    RSRP["id"] = []

    with torch.no_grad():
        for idx, (features, image, PL, id) in enumerate(data_loader):
            if args.cuda:
                features = features.cuda()
                image = image.cuda()
                PL = PL.cuda()

            output = model(image, features)

            RSRP["DNN"] += (torch.tensor([elem[0] for elem in target_scaler.inverse_transform(output.cpu().numpy())]) + PL.cpu()).tolist()
            RSRP["id"] += list(id.cpu().numpy())

            bar.update(1)
    
    bar.close()

    return RSRP



def run(_args):
    if _args.cuda and torch.cuda.is_available():
        cuda = True
        print("CUDA enabled")
    else:
        cuda = False
        print("CUDA disabled")

    # Load experiment
    folder = _args.folder
    path = _args.path
    id = _args.id
    exp = load_experiment(_args.exps, root_path=folder)
    name = _args.exps
    args = edict(exp.config)
    args.name = name.split('_')[0]
    args.cuda = cuda
    args.base_path = _args.base_path

    try: 
        args.areas
    except AttributeError:
        args.areas = _args.areas

    try: 
        args.folders
    except AttributeError:
        args.folders = _args.folders

    try: 
        args.heights
    except AttributeError:
        args.heights = _args.heights

    # get Data
    _, _, _, input_scaler, target_scaler, columns = process_data.process_data(args, seed=args.seed)
    samples = process_data.process_data_REM(path + id + 'features/feature.pkl', args) 

    _tmp = id.split('/')[-2]
    _path = id
    dataset = MyDataset(args.base_path, samples, input_scaler, target_scaler, args.image_size, args.image_range, columns, args.im_model, args.num_workers, args.load_images, args.sv, args.tv, args.num_workers, REM=True, path=path + _path)

    # load data
    data_loader = torch.utils.data.DataLoader(dataset, batch_size=args.batch_size, shuffle=False, num_workers=args.num_workers, drop_last=False)

    # init model
    model = model_dragon.CombiModel(args, target_scaler)

    if args.cuda:
        model.cuda()

    # find model name
    list_of_files = os.listdir(folder + 'models/')
    for filename in list_of_files:
        if filename.startswith(args.name):  
            model_name = filename
            print(model_name)
            break
    
    # load model state
    if args.cuda:
        model.load_state_dict(torch.load(folder + 'models/' + model_name))
    else:
        model.load_state_dict(torch.load(folder + '/models/' + model_name, map_location=torch.device('cpu')))
    model.eval()

    # evaluate test data
    RSRP = predict(args, data_loader, model, target_scaler)
    
    # save results
    RSRP_df = pd.DataFrame.from_dict(data=RSRP, orient='columns')
    RSRP_df.to_csv(folder + 'REM/' + id.split('/')[4] + '/' + id.split('/')[2] + '_' + id.split('/')[3] + '_' + id.split('/')[5] + '.csv')  


if __name__ == '__main__':
    args = init_model.init_args()

    args.folder = args.base_path + 'models/results/exps/'
    args.path = args.base_path + 'dataset_REM/'

    for folder in os.listdir(args.path + args.REM_folder)[6::]:
        if os.path.isdir(args.path + args.REM_folder + folder):
            args.id = args.REM_folder + folder + '/'
            run(args)

