from PIL import Image

image = Image.open("/Users/jessup/Desktop/scrooge.bmp")
(cols, rows) = image.size

print 'char scroogeImg[][6] = {'

for col in range(0,cols):
    vals = []
    bin = 0x0;
    for row in range(0, rows):
        bin = bin + image.getpixel((col, row))
        if ((row+1) % 8 == 0):
            vals.append(hex(bin))
            bin = 0x0;
        else:
            bin = bin << 1
            
    print '  {'+', '.join(vals)+'},'

print '};'