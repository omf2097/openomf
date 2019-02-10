# -*- coding: utf-8 -*-

import csv
import sqlite3

tags_file = open('tags.csv', 'rb')
conn = sqlite3.connect('taglist.sqlite')
c = conn.cursor()


# Who wants to handle errors anyways ?
try:
    c.execute('DROP TABLE tags')
except:
    pass
c.execute('CREATE TABLE tags (tag varchar(3) UNIQUE PRIMARY KEY, has_param int, description text)')


reader = csv.reader(tags_file)
for row in reader:
    out = [row[0], int(row[1]), str(row[2])]
    c.execute("INSERT INTO tags (tag,has_param,description) VALUES (?,?,?)", out)
conn.commit()
    
conn.close()
tags_file.close()