#!/usr/bin/env python3

import os
import pandas as pd
import datetime
import matplotlib.dates as mdates
import math


os.chdir("./raw_data")


for file_name_str in sorted(os.listdir("./")):
    is_open_door = False
    is_come_in   = False
    behaving_count = 0
    behaving_time_lst  = list()
    timestamp_lst = list()
    exception_time_lst = list()

    date_str = file_name_str.split('.')[0]
    #date_str = date_str.split('-')[1] + "-" + date_str.split('-')[2]

    df = pd.read_json(file_name_str, lines=True) #生データの取得
    df = df.drop_duplicates() #重複行の削除
    #df = df.set_index('timestamp') #timestampをインデックス化

    df.sort_index(inplace=True) # 時系列化
    # print(len(df[(df['location'] == "toilet_room") & (df['target'] == "door")])) #ドアの開閉回数
    data_df = df[(df['location'] == "toilet_room") & (df['target'] == "door")]

    for i in range(len(data_df)):
        if int(data_df.iloc[i]['door']) == 1 and not is_open_door:
            #print("トイレに入った")
            is_open_door = True
            timestamp_of_come_in = data_df.iloc[i]['timestamp']
            
        elif int(data_df.iloc[i]['door']) == 0 and is_open_door:
            #print("トイレから出た")
            is_open_door = False
            timestamp_of_come_out = data_df.iloc[i]['timestamp']

            if 5 <= (timestamp_of_come_out - timestamp_of_come_in).total_seconds() <= 60*60:
                #print("トイレ時間:", (timestamp_of_come_out - timestamp_of_come_in).total_seconds())
                #print()
                timestamp_lst.append({
                    "start": timestamp_of_come_in.strftime('%H:%M:%S'),
                    "end": timestamp_of_come_out.strftime('%H:%M:%S'),
                    "time": int((timestamp_of_come_out - timestamp_of_come_in).total_seconds())
                })
                behaving_count += 1
                behaving_time_lst.append((timestamp_of_come_out - timestamp_of_come_in).total_seconds())

            else:
                #print("例外:", (timestamp_of_come_out - timestamp_of_come_in).total_seconds())
                exception_time_lst.append((timestamp_of_come_out - timestamp_of_come_in).total_seconds())

         
    with open('../data/' + 'toilet.jsonl', 'a') as fpw:
        fpw.write( 
            str({
                "uid": 1,
                "activity": "toilet",
                "time": int(sum(behaving_time_lst)),
                "count": behaving_count,
                "date": file_name_str.split('.')[0],
                "timestamp": timestamp_lst
            }) + '\n'
        )
