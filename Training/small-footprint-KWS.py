import torch
import os
import numpy as np
import librosa
import matplotlib.pyplot as plt
import torch.nn as nn
import torch.nn.functional as F
from torch import optim

num_classes = 6
num_commands = 4
selected_commands = ["yes", "no", "left", "right"]

spec_t = 30
spec_f = 40

num_train_samples = 2000
num_test_samples = 350

num_train_samples_unselected = 77
num_test_samples_unselected = 14

class SpeechCmdDataset(torch.utils.data.Dataset):

    def log_mel_spec(self, file_path, sample_rate, window_len, n_mels):
        signal = librosa.load(file_path, sr=sample_rate)[0]

        signal = signal[0:window_len]
        signal = np.pad(signal, (0, window_len - signal.shape[0]), 'constant')

        melspec = librosa.feature.melspectrogram(y=signal, sr=sample_rate, n_mels=n_mels, n_fft=1024,
                                                 hop_length=512, center=False)

        log_melspec = librosa.power_to_db(melspec) * 2
        log_melspec = log_melspec[np.newaxis, :]

        return log_melspec

    def __init__(self, dataset_dir, cache_dir, sample_rate=16000, n_mels=spec_f, n_frames=spec_t, is_train=True):

        self.label_names = ["background", "unknown"]
        self.word_spec = []
        self.word_label = []
        self.num_word_specs = 0
        self.background_spec = []
        self.num_backgrounds = 0

        if is_train:
            self.num_backgrounds_balanced = num_train_samples
        else:
            self.num_backgrounds_balanced = num_test_samples

        if os.path.exists(cache_dir):
            self.word_spec = np.load(os.path.join(cache_dir, "word_spec.npy"))
            self.word_label = np.load(os.path.join(cache_dir, "word_label.npy"))
            self.num_word_specs = self.word_label.shape[0]
            self.background_spec = np.load(os.path.join(cache_dir, "background_spec.npy"))
            self.num_backgrounds = self.background_spec.shape[0]

            with open(os.path.join(cache_dir, "label_names.txt"), "r") as f:
                self.label_names = [line.rstrip() for line in f]
            return


        window_len = 512 * (n_frames + 1)

        command_and_unknown_spec = []
        command_and_unknown_labels = []

        background_spec = []

        cnt_all_unknown = 0

        for command_word in os.listdir(dataset_dir):

            if not os.path.isdir(os.path.join(dataset_dir, command_word)):
                continue

            # Selected words
            if command_word in selected_commands:
                self.label_names.append(command_word)
                cnt_cur_command = 0

                for file_name in os.listdir(os.path.join(dataset_dir, command_word)):

                    if is_train:
                        if cnt_cur_command >= num_train_samples:
                            break
                        cnt_cur_command += 1

                    else:
                        if cnt_cur_command < num_train_samples:
                            cnt_cur_command += 1
                            continue

                    command_and_unknown_spec.append(
                        self.log_mel_spec(os.path.join(dataset_dir, command_word, file_name), sample_rate, window_len, n_mels))
                    command_and_unknown_labels.append(len(self.label_names) - 1)

            # Background noise
            elif command_word == "_background_noise_":

                for file_name in os.listdir(os.path.join(dataset_dir, command_word)):

                    if file_name == "README.md":
                        continue
                    background_spec.append(self.log_mel_spec(os.path.join(dataset_dir, command_word, file_name),
                                                             sample_rate, window_len, n_mels))

            # Unselected words
            else:

                cnt_cur_command = 0

                for file_name in os.listdir(os.path.join(dataset_dir, command_word)):

                    if is_train:

                        if cnt_cur_command >= num_train_samples_unselected:
                            break
                        cnt_cur_command += 1

                        if cnt_all_unknown >= num_train_samples:
                            break
                        cnt_all_unknown += 1

                    else:

                        if cnt_cur_command < num_train_samples_unselected:
                            cnt_cur_command += 1
                            continue

                        elif cnt_cur_command >= num_train_samples_unselected and (cnt_cur_command <
                                num_train_samples_unselected + num_test_samples_unselected):
                            cnt_cur_command += 1
                        else:
                            break

                        if cnt_all_unknown >= num_test_samples:
                            break
                        cnt_all_unknown += 1

                    command_and_unknown_spec.append(
                        self.log_mel_spec(os.path.join(dataset_dir, command_word, file_name), sample_rate, window_len,
                                          n_mels))
                    command_and_unknown_labels.append(1)

        self.word_spec = np.asarray(command_and_unknown_spec)
        self.word_label = np.asarray(command_and_unknown_labels)
        self.num_word_specs = len(command_and_unknown_labels)

        self.background_spec = np.asarray(background_spec)
        self.num_backgrounds = len(background_spec)

        if not os.path.exists(cache_dir):
            os.mkdir(cache_dir)
            np.save(os.path.join(cache_dir, "word_spec.npy"), self.word_spec)
            np.save(os.path.join(cache_dir, "word_label.npy"), self.word_label)
            np.save(os.path.join(cache_dir, "background_spec.npy"), self.background_spec)
            with open(os.path.join(cache_dir, "label_names.txt"), "w") as f:
                for name in self.label_names:
                    f.write(name)
                    f.write('\n')


    def __len__(self):
        return self.num_word_specs + self.num_backgrounds_balanced

    def __getitem__(self, index):
        if index < self.num_word_specs:
            return self.word_spec[index], self.word_label[index]
        index -= self.num_word_specs
        index %= self.num_backgrounds
        return self.background_spec[index], 0

asc_train_dataset = SpeechCmdDataset("./data/speech_commands", "./prep_train")
asc_train_dataloader = torch.utils.data.DataLoader(asc_train_dataset, batch_size=12, shuffle=True)

asc_test_dataset = SpeechCmdDataset("./data/speech_commands", "./prep_test", is_train=False)
asc_test_dataloader = torch.utils.data.DataLoader(asc_test_dataset, batch_size=12, shuffle=False)


class_names = asc_train_dataset.label_names


def show_specs(images, labels, preds):
    plt.figure(figsize=(8, 4))
    for i, image in enumerate(images):
        plt.subplot(1, 6, i + 1, xticks=[], yticks=[])
        plt.imshow(image[0])
        col = 'green'
        if preds[i] != labels[i]:
            col = 'red'

        plt.xlabel(f'{class_names[int(labels[i].numpy())]}')
        plt.ylabel(f'{class_names[int(preds[i].numpy())]}', color=col)
    plt.tight_layout()
    plt.show()


specs, spec_labels = next(iter(asc_train_dataloader))
show_specs(specs[:6], spec_labels[:6], spec_labels[:6])
show_specs(specs[6:], spec_labels[6:], spec_labels[6:])


class Net(nn.Module):

    def __init__(self):
        super().__init__()

        self.conv = nn.Conv2d(1, 186, (8, spec_t), (4, 1))

        # self.lin = nn.Linear(9 * 186, 32, bias=False)
        # self.fc1 = nn.Linear(32, 128)
        # self.fc2 = nn.Linear(128, 128)
        # self.fc3 = nn.Linear(128, num_classes)

        self.fc1 = nn.Linear(9 * 186, 32)
        self.fc2 = nn.Linear(32, 128)
        self.fc3 = nn.Linear(128, 128)
        self.fc4 = nn.Linear(128, num_classes)

        self.dropout = nn.Dropout(0.2)

    def forward(self, in_feature):

        for sample in in_feature:
            sample -= torch.mean(sample)
            sample /= torch.std(sample)

        x = self.conv(in_feature)
        # x = self.dropout(x)
        x = F.relu(x)

        x = torch.flatten(x, 1)  # flatten all dimensions except batch

        # x = self.lin(x)
        # x = self.dropout(x)
        x = self.fc1(x)
        x = F.relu(x)

        x = self.fc2(x)
        # x = self.dropout(x)
        x = F.relu(x)

        x = self.fc3(x)
        # x = self.dropout(x)
        x = F.relu(x)

        x = self.fc4(x)

        return x

model = Net()
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters())

num_epochs = 40

def val():

    model.eval()
    with torch.no_grad():

        accuracy = 0
        val_loss = 0
        for val_step, (inputs, labels) in enumerate(asc_test_dataloader):
            outputs = model(inputs)
            labels = torch.tensor(labels, dtype=torch.long)
            loss = criterion(outputs, labels)
            val_loss += loss.item()

            _, preds = torch.max(outputs, 1)
            accuracy += (preds == labels).sum()

        val_loss /= (val_step + 1)
        accuracy = accuracy / len(asc_test_dataset)
        print(f'Validation Loss: {val_loss:.4f}, Accuracy: {accuracy:.4f}')
    model.train()

for epoch in range(num_epochs):  # loop over the dataset multiple times

    running_loss = 0.0
    correct = 0
    for i, data in enumerate(asc_train_dataloader, 0):
        # get the inputs; data is a list of [inputs, labels]
        inputs, labels = data
        labels = torch.tensor(labels, dtype=torch.long)

        # zero the parameter gradients
        optimizer.zero_grad()

        # forward + backward + optimize
        outputs = model(inputs)
        loss = criterion(outputs, labels)
        loss.backward()

        optimizer.step()

        _, preds = torch.max(outputs, 1)
        # show_images(inputs[:6], labels[:6], preds[:6])
        correct += (preds == labels).sum()

        # print statistics
        running_loss += loss.item()
        if i % 1 == 250:    # print every 2000 mini-batches
            print('Epoch [{}/{}], Step [{}/{}], Loss(Avg): {:.3f}'
                  .format(epoch + 1, num_epochs, i + 1, len(asc_train_dataloader), running_loss / 1))
            running_loss = 0.0
    print("Epoch {}, Acc: {}/{}".format(epoch + 1, correct, len(asc_train_dataset)))
    val()

print(0)

torch.save(model.state_dict(), "cnn-one-fstride4.pt")

def print_state_dict():
    # Print model's state_dict
    print("Model's state_dict:")
    for param_tensor in model.state_dict():
        print(param_tensor, "\t", model.state_dict()[param_tensor].size())

def print_optim_state_dict():
    # Print optimizer's state_dict
    print("Optimizer's state_dict:")
    for var_name in optimizer.state_dict():
        print(var_name, "\t", optimizer.state_dict()[var_name])


def printConvW(varname, weight_data, header=False):
    if header:
        print("extern ", end="")
    print("float {}[{}][{}][{}][{}] = ".format(varname, weight_data.shape[0],weight_data.shape[1],weight_data.shape[2],weight_data.shape[3],))

    if header:
        print(";")
        return

    print("{\n", end="")
    for outch in weight_data:
        print("\t{\n", end="")
        for inch in outch:
            print("\t\t{\n", end="")
            for row in inch:
                print("\t\t\t{", end="")
                for col in row:
                    print(float(col), end=", ")
                print("},\n", end="")
            print("\t\t},\n", end="")
        print("\t},\n", end="")
    print("}", end=";\n")

def printFcW(varname, weight_data, header=False):
    if header:
        print("extern ", end="")
    print("float {}[{}][{}] = ".format(varname, weight_data.shape[0], weight_data.shape[1]))
    if header:
        print(";")
        return
    print("{\n", end="")
    for outch in weight_data:
        print("\t{", end="")
        for inch in outch:
            print(float(inch), end=", ")
        print("},\n", end="")
    print("}", end=";\n")

def printBiasW(varname, bias_data, header=False):
    if header:
        print("extern ", end="")
    print("float {}[{}] = ".format(varname, bias_data.shape[0]))
    if header:
        print(";")
        return
    print("\t{", end="")
    for outch in bias_data:
        print(float(outch), end=", ")
    print("}", end=";\n")

def export_state_dict_C(code_file, header=False):
    # Print model's state_dict
    print("Model's state_dict in C lang:")

    import sys
    savedStdout = sys.stdout  # 保存标准输出流
    with open(code_file, 'w+') as file:
        sys.stdout = file  # 标准输出重定向至文件

        for param_tensor in model.state_dict():
            if model.state_dict()[param_tensor].data.numpy().shape.__len__() == 4:
                printConvW(param_tensor.replace('.', '_'), model.state_dict()[param_tensor].data.numpy(), header)
            if model.state_dict()[param_tensor].data.numpy().shape.__len__() == 2:
                printFcW(param_tensor.replace('.', '_'), model.state_dict()[param_tensor].data.numpy(), header)
            if model.state_dict()[param_tensor].data.numpy().shape.__len__() == 1:
                printBiasW(param_tensor.replace('.', '_'), model.state_dict()[param_tensor].data.numpy(), header)

        sys.stdout = savedStdout  # 恢复标准输出流

def show_kernels():
    for i in range(0, 31):
        show_specs(model.state_dict()["conv.weight"][i:i + 6], torch.from_numpy(np.zeros(6, dtype=int)),
                   torch.from_numpy(np.zeros(6, dtype=int)))