import os
import torch
import wandb

import numpy as np

from types import SimpleNamespace
from experimentlogger import Experiment
from torch import nn
from torch import optim
from tqdm import tqdm
from torch.optim import optimizer
from tqdm.std import trange

import helpers_ML
import MultiEpochsDataLoader
import init_model
import model_dragon
import process_data

from dataset import MyDataset



def train(args, train_loader, optimizer, loss_function, model, target_scaler):
    bar = tqdm(total = len(train_loader))
    total_loss = 0

    sum = 0
    ctr = 0

    for idx, (features, image, target) in enumerate(train_loader):
        if args.cuda:
            features = features.cuda()
            image = image.cuda()
            target = target.cuda()

        # reset gradients
        optimizer.zero_grad()

        output = model(image, features)
        loss = loss_function(output, target)
        loss.backward()

        total_loss += loss.item()
        optimizer.step()
        
        bar.update(1)

        output = target_scaler.inverse_transform(output.cpu().detach())
        target = target_scaler.inverse_transform(target.cpu().detach())
            
        sum += np.sum(np.abs(target - output))
        ctr += len(target)
    
    bar.close()
    return total_loss / len(train_loader), sum / ctr



def val(args, val_loader, loss_function, model, target_scaler):
    bar = tqdm(total = len(val_loader))
    total_loss = 0

    sum = 0
    ctr = 0
    
    with torch.no_grad():
        for features, image, target in val_loader:
            if args.cuda:
                features = features.cuda()
                image = image.cuda()
                target = target.cuda()

            output = model(image, features)

            loss = loss_function(output, target)
            total_loss += loss.item()

            bar.update(1)

            output = target_scaler.inverse_transform(output.cpu().detach())
            target = target_scaler.inverse_transform(target.cpu().detach())
                
            sum += np.sum(np.abs(target - output))
            ctr += len(target)
    
    bar.close()
    return total_loss / len(val_loader), sum / ctr


def trainLoop(args):
    # get data
    train_set, val_set, test_set, input_scaler, target_scaler, columns = process_data.process_data(args, seed=args.seed)
    args.columns = columns
    
    # load data
    loader_class = torch.utils.data.DataLoader
    if args.use_multi_epochs_loader:
        loader_class = MultiEpochsDataLoader.MultiEpochsDataLoader

    train_dataset = MyDataset(args.base_path, train_set, input_scaler, target_scaler, args.image_size, args.image_range, columns, args.im_model, args.data_augmentation, args.load_images, args.sv, args.tv, args.num_workers, mapped_heights=args.mapped_heights, corrected_position=args.corrected_position)
    val_dataset = MyDataset(args.base_path, val_set, input_scaler, target_scaler, args.image_size, args.image_range, columns, args.im_model, 0, args.load_images, args.sv, args.tv, args.num_workers, mapped_heights=args.mapped_heights, corrected_position=args.corrected_position)
    train_loader = loader_class(train_dataset, batch_size=args.batch_size, shuffle=True, num_workers=args.num_workers, drop_last=True)
    val_loader = loader_class(val_dataset, batch_size=args.batch_size, shuffle=False, num_workers=args.num_workers, drop_last=False)

    if args.test:
        test_dataset = MyDataset(args.base_path, test_set, input_scaler, target_scaler, args.image_size, args.image_range, columns, args.im_model, 0, args.load_images, args.sv, args.tv, args.num_workers, mapped_heights=args.mapped_heights, corrected_position=args.corrected_position)
        test_loader = loader_class(test_dataset, batch_size=args.batch_size, shuffle=False, num_workers=args.num_workers, drop_last=False)
    else:
        del test_set
    
    
    # init model
    args.num_features = train_dataset.features.shape[1]
    args.in_channels = 1
    model = model_dragon.CombiModel(args, target_scaler)
    
    if args.wandb:
        wandb.watch(model)

    if args.cuda:
        model.cuda()

    # define params
    loss_function = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=args.learning_rate, weight_decay=args.weight_decay)
    scheduler_model = optim.lr_scheduler.ReduceLROnPlateau(optimizer, patience=10)

    # start training
    train_loss = []
    train_MAE = []
    val_loss = []
    val_MAE = []
    test_loss = []
    test_MAE = []


    for epoch in range(args.epochs):
        model.train()
        loss, MAE = train(args, train_loader, optimizer, loss_function, model, target_scaler)
        train_loss.append(loss)
        train_MAE.append(MAE)

        model.eval()
        loss, MAE = val(args, val_loader, loss_function, model, target_scaler)
        val_loss.append(loss)
        val_MAE.append(MAE)

        if args.test:
            loss, MAE = val(args, test_loader, loss_function, model, target_scaler)
            test_loss.append(loss)
            test_MAE.append(MAE)

        scheduler_model.step(val_loss[-1])

        if args.test:
            if args.wandb:
                wandb.log({"val_loss": val_loss[-1], "train_loss": train_loss[-1], "train val gap": train_loss[-1]-val_loss[-1], "train_MAE": train_MAE[-1], "val_MAE": val_MAE[-1], "train test gap": train_loss[-1]-test_loss[-1], "test_MAE": test_MAE[-1], "test_loss": test_loss[-1]})
            text = "Epoch: {}, train_loss: {}, val_loss: {}, test_loss: {}, train_MAE: {}, val_MAE: {}, test_MAE: {}".format(epoch, train_loss[-1], val_loss[-1], test_loss[-1], train_MAE[-1], val_MAE[-1], test_MAE[-1])
        else:
            if args.wandb:
                wandb.log({"val_loss": val_loss[-1], "train_loss": train_loss[-1], "train val gap": train_loss[-1]-val_loss[-1], "train_MAE": train_MAE[-1], "val_MAE": val_MAE[-1]})
            text = "Epoch: {}, train_loss: {}, val_loss: {}, train_MAE: {}, val_MAE: {}".format(epoch, train_loss[-1], val_loss[-1], train_MAE[-1], val_MAE[-1])
        print("\n" + text)

        if optimizer.param_groups[0]['lr'] < 1e-7:
            print('Learning rate too low. Early stopping.')
            break
    
    # save model parameter
    exp = Experiment('file', config=args.__dict__, root_folder=args.base_path + 'models/results/exps')
    results_dict = dict()
    results_dict['train_loss'] = train_loss
    results_dict['train_MAE'] = train_MAE
    results_dict['val_loss'] = val_loss
    results_dict['val_MAE'] = val_MAE
    if args.test:
        results_dict['test_loss'] = test_loss
        results_dict['test_MAE'] = test_MAE
    exp.results = results_dict
    exp.save()

    # save trained model
    try:
        torch.save(model.state_dict(), exp.root_folder+'/models/{}_model_{:.3f}.pt'.format(exp.id,val_loss[-1]))
    except FileNotFoundError:
        print('File not found - Error')
    print('training ended')


def run(args):
    torch.manual_seed(args.seed)
    if args.cuda and torch.cuda.is_available():
        print('CUDA enabled')
        torch.cuda.empty_cache()
        torch.cuda.manual_seed(args.seed)
    else:
        print('CUDA not available')
        args.cuda = False
    
    if args.wandb:
        wandb.init(project=args.wandb_project, config=args)
    trainLoop(args)


if __name__ == '__main__':
    args = init_model.init_args()
    run(args)