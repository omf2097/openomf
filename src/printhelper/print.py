from __future__ import print_function
import os
import argparse
import subprocess

parser = argparse.ArgumentParser(description='Produce HTML output of BK and AF files')
parser.add_argument('-i', metavar='<input>', required=True, nargs=1, help='Resource input path')
parser.add_argument('-o', metavar='<output>', required=True, nargs=1, help='Html output path')
args = parser.parse_args()
input_path = args.i[0]
output_path = args.o[0]

if not os.path.isdir(input_path):
    print("Input path is not a valid directory!")
    exit(1)
if not os.path.isdir(output_path):
    print("Output path is not a valid directory!")
    exit(1)

bk_files = []
af_files = []

print("Traversing path %s" % (input_path,))
for root, dirs, files in os.walk(input_path):
    for file in files:
        if file.endswith(".BK"):
            bk_files.append(os.path.join(input_path,file))
        if file.endswith(".AF"):
            af_files.append(os.path.join(input_path,file))
print("Found %d AF files and %d BK files." % (len(bk_files), len(af_files),))

palette_bk = None
for mfile in bk_files:
    if os.path.basename(mfile) == "ARENA0.BK":
        palette_bk = mfile

if palette_bk == None:
    print("No ARENA0.BK found for palette.")
    exit(1)

if len(bk_files) == 0 and len(af_files) == 0:
    print("No files found, stopping.")
    exit(1)

output_af_files = []
output_bk_files = []

for mfile in bk_files:
    filebase = os.path.basename(mfile)
    output_file = os.path.splitext(filebase)[0]
    output_file = output_file.lower()
    output_bk_files.append((output_file, filebase,))
    print("Parsing %s to %s ... " % (filebase,output_file+".html",), end='')
    
    try:
        subprocess.call(["bkhtmlprinter", "-f", mfile, "-o", output_path, "-n", output_file])
    except:
        print("Error!")
        exit(1)

    print("OK")

for mfile in af_files:
    filebase = os.path.basename(mfile)
    output_file = os.path.splitext(filebase)[0]
    output_file = output_file.lower()
    output_af_files.append((output_file, filebase,))
    print("Parsing %s to %s ... " % (filebase,output_file+".html",), end='')
    
    try:
        subprocess.call(["afhtmlprinter", "-f", mfile, "-p", palette_bk, "-o", output_path, "-n", output_file])
    except:
        print("Error!")
        exit(1)

    print("OK")

print("Creating index")
f = open(os.path.join(output_path, "index.html"), 'wb')
f.write("<html><head><title>OMF:2097 Data files</title></head><body><h1>OMF:2097 Data files</h1>")

f.write("<h2>Fighters:</h2><ul>")
for mfile in output_af_files:
    f.write('<li><a href="%s.html">%s</a></li>' % mfile)
f.write("</ul>")

f.write("<h2>Scenes:</h2><ul>")
for mfile in output_bk_files:
    f.write('<li><a href="%s.html">%s</a></li>' % mfile)
f.write("</ul>")

f.write("</body></html>")
f.close()

print("All done!")