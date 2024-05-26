import torch
from torch.nn import Linear, ReLU, Softmax
from torch_geometric.nn import SAGEConv, GraphConv

class ClauseRec(torch.nn.Module):
    def __init__(self, hidden_channels):
        super(ClauseRec, self).__init__()
        self.conv1 = SAGEConv((-1,-1), hidden_channels)
        self.conv2 = SAGEConv((-1,-1), hidden_channels)
        self.conv3 = GraphConv(hidden_channels,hidden_channels)
        self.linear = Linear(hidden_channels, 1)
        self.relu = ReLU(inplace=True)
        self.softmax = Softmax(dim=1)
        
    def forward(self, x, edge_index):
        x = self.conv1(x, edge_index)
        x = self.relu(x)
        x = self.conv2(x, edge_index)
        x = self.relu(x)
        x = self.conv3(x, edge_index)
        x = self.relu(x)
        x = self.linear(x)
        return self.softmax(x)