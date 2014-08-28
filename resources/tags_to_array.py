# -*- coding: utf-8 -*-

import csv

tags_file = open('tags.csv', 'rb')
output_file = open('../src/taglist.c','wb')

output_file.write('#include <stdlib.h>\n')
output_file.write('#include "shadowdive/taglist.h"\n\n')
output_file.write('// This file is generated automatically\n\n')
output_file.write('static const sd_tag sd_tags[] = {\n')
reader = csv.reader(tags_file)
for row in reader:
    output_file.write("    {")
    output_file.write('"%s", ' % row[0])
    output_file.write('%s, ' % row[1])
    if len(row[2]) > 0:
        output_file.write('"%s"' % row[2])
    else:
        output_file.write('NULL')
    output_file.write('},\n')
    
    
output_file.write('};\n')
output_file.close()
tags_file.close()