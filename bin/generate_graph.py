#!/usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np
import sys
import re
import csv


def _parse_data(file_path: str, column: str):
    ret=[]
    with open(file_path, newline='') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter=',')
        for line in csv_reader:
            ret.append(line[column])
    return np.array(ret).astype(np.float64  )


def generate_graph():
    if len(sys.argv) < 2:
        print("Il faut au moins un nom de fichier")
        return 1
    joined_data = []
    joined_labels = []
    for file_path in sys.argv[1::]:
        data = _parse_data(file_path=file_path, column="real_s")
        joined_data.append(data)
        p = re.compile("([a-zA-Z_].*)_(Debug|Release)_([0-9]*)its")
        m = p.match(file_path)
        joined_labels.append(m.group(1))
        
    print(joined_data)
    fig1, ax1 = plt.subplots()
    ax1.set_title(m.group(2) + " " + m.group(3) + " itérations")
    ax1.boxplot(joined_data, labels=joined_labels)
    
    plt.savefig("graph.png")
    plt.show()


generate_graph()
