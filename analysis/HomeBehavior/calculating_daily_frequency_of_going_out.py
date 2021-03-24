#!/usr/bin/env python3

import os
import pandas as pd
import datetime
import matplotlib.dates as mdates
import math


os.chdir("./raw_data")


for file_name_str in sorted(os.listdir("./")):
    is_open_door = False
    is_go_out    = False
    behaving_count = 0
    behaving_time_lst  = list()
    timestamp_lst = list()

    date_str = file_name_str.split('.')[0]
    #date_str = date_str.split('-')[1] + "-" + date_str.split('-')[2]

    df = pd.read_json(file_name_str, lines=True) #生データの取得
    df = df.drop_duplicates() #重複行の削除
    #df = df.set_index('timestamp') #timestampをインデックス化

    df.sort_index(inplace=True) # 時系列化
    # print(len(df[(df['location'] == "front_entrance") & (df['target'] == "door")])) #ドアの開閉回数
    data_df = df[(df['location'] == "front_entrance") & (df['target'] == "door")]
    

    for i in range(len(data_df)):
        if int(data_df.iloc[i]['door']) == 1 and not is_open_door and not is_go_out:
            ### 宅内にいる状態でドアが開いた ###
            from_closed_door_data = df.loc[ (df['timestamp'] < data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] >= data_df.iloc[i]['timestamp'] - datetime.timedelta(seconds=600) )]
            
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) > 0:
                is_open_door = True

        elif int(data_df.iloc[i]['door']) == 0 and is_open_door and not is_go_out:
            ### 宅内にいる状態でドアが閉じた ###
            from_closed_door_data = df.loc[ (df['timestamp'] < data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] >= data_df.iloc[i]['timestamp'] - datetime.timedelta(seconds=600) )]
            
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) > 0:
                is_open_door = False

            from_closed_door_data = df.loc[ (df['timestamp'] > data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] <= data_df.iloc[i]['timestamp'] + datetime.timedelta(seconds=600) )]
            
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) == 0:
                ### 外出した ###
                is_go_out = True
                timestamp_of_go_out = data_df.iloc[i]['timestamp']
            
        elif int(data_df.iloc[i]['door']) == 1 and not is_open_door and is_go_out:
            ### 外出している状態でドアが開いた ###
            from_closed_door_data = df.loc[ (df['timestamp'] < data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] >= data_df.iloc[i]['timestamp'] - datetime.timedelta(seconds=600) )]
            
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) == 0:
                is_open_door = True

        elif int(data_df.iloc[i]['door']) == 0 and is_open_door and is_go_out:
            ### 外出している状態でドアが閉じた ###
            from_closed_door_data = df.loc[ (df['timestamp'] < data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] >= data_df.iloc[i]['timestamp'] - datetime.timedelta(seconds=600) )]      
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) > 0:
                is_open_door = False

            from_closed_door_data = df.loc[ (df['timestamp'] > data_df.iloc[i]['timestamp']) 
                & (df['timestamp'] <= data_df.iloc[i]['timestamp'] + datetime.timedelta(seconds=600) )]

            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) == 0:
                ### 外出した ###
                timestamp_of_go_out = data_df.iloc[i]['timestamp']
 
            if len(from_closed_door_data.loc[from_closed_door_data['subject'] == 1]) > 0:
                ### 帰宅した ###
                is_go_out = False
                timestamp_of_go_back = data_df.iloc[i]['timestamp']
                
                timestamp_lst.append({
                    "start": timestamp_of_go_out.strftime('%H:%M:%S'),
                    "end": timestamp_of_go_back.strftime('%H:%M:%S'),
                    "time": int((timestamp_of_go_back - timestamp_of_go_out).total_seconds())
                })
                behaving_count += 1
                behaving_time_lst.append((timestamp_of_go_back - timestamp_of_go_out).total_seconds())


    with open('../data/' + 'going_out.jsonl', 'a') as fpw:
        fpw.write( 
            str({
                "uid": 1000,
                "activity": "going_out",
                "time": int(sum(behaving_time_lst)),
                "count": behaving_count,
                "date": file_name_str.split('.')[0],
                "timestamp": timestamp_lst
            }) + '\n'
        )
