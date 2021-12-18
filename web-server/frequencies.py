import pandas as pd 
import numpy as np 


df = pd.read_csv("frequencies.csv",index_col=False)
global freqlist
global notelist
freqlist = df['Frequency'].values
notelist = df['Note'].values
