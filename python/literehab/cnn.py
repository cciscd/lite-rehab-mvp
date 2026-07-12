from __future__ import annotations


def build_model(num_classes: int, arch: str = "cnn_bigru"):
    import torch.nn as nn

    if arch == "cnn_1d":
        return _build_1dcnn(num_classes)
    if arch == "cnn_bigru":
        return _build_cnn_bigru(num_classes)
    raise ValueError(f"unknown architecture: {arch}")


def _build_1dcnn(num_classes: int):
    import torch.nn as nn

    class IMU1DCNN(nn.Module):
        def __init__(self) -> None:
            super().__init__()
            self.features = nn.Sequential(
                nn.Conv1d(6, 32, kernel_size=5, padding=2),
                nn.ReLU(),
                nn.MaxPool1d(2),
                nn.Conv1d(32, 64, kernel_size=5, padding=2),
                nn.ReLU(),
                nn.AdaptiveAvgPool1d(1),
            )
            self.classifier = nn.Linear(64, num_classes)

        def forward(self, x):
            return self.classifier(self.features(x).squeeze(-1))

    return IMU1DCNN()


def _build_cnn_bigru(num_classes: int):
    import torch.nn as nn

    class IMUCNNBiGRU(nn.Module):
        def __init__(self) -> None:
            super().__init__()
            self.conv1 = nn.Sequential(
                nn.Conv1d(6, 64, kernel_size=3, padding=1),
                nn.BatchNorm1d(64),
                nn.ReLU(),
                nn.MaxPool1d(2),
            )
            self.conv2 = nn.Sequential(
                nn.Conv1d(64, 128, kernel_size=3, padding=1),
                nn.BatchNorm1d(128),
                nn.ReLU(),
                nn.MaxPool1d(2),
            )
            self.conv3 = nn.Sequential(
                nn.Conv1d(128, 256, kernel_size=3, padding=1),
                nn.BatchNorm1d(256),
                nn.ReLU(),
            )
            self.bigru = nn.GRU(256, 128, batch_first=True,
                                bidirectional=True)
            self.dropout = nn.Dropout(0.5)
            self.classifier = nn.Linear(256, num_classes)

        def forward(self, x):
            x = self.conv1(x)
            x = self.conv2(x)
            x = self.conv3(x)
            x = x.permute(0, 2, 1)
            x, _ = self.bigru(x)
            x = x.mean(dim=1)
            x = self.dropout(x)
            return self.classifier(x)

    return IMUCNNBiGRU()
