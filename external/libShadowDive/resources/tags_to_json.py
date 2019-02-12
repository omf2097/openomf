# -*- coding: utf-8 -*-

import csv
import json

tags_file = open('tags.csv', 'rb')
output_file = open('taglist.json','wb')

tags = []
reader = csv.reader(tags_file)
for row in reader:
    tags.append((row[0], int(row[1]), str(row[2])))

output_file.write(json.dumps(tags))
    
output_file.close()
tags_file.close()