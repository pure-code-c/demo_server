import os
import random
import base64
import sys
from io import BytesIO
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

#随机底色
def random_color():
    c1 = random.randint(0, 255)
    c2 = random.randint(0, 255)
    c3 = random.randint(0, 255)
    return c1, c2, c3

#产生图片对象
def generate_picture(width=120, height=35):
    image = Image.new('RGB', (width, height), random_color())
    return image

#绘制字符
def draw_str(word, image, font_size):
    draw = ImageDraw.Draw(image)
    # 获取一个font字体对象参数是ttf的字体文件的目录，以及字体的大小
    font_file = os.path.join('TlwgTypo-Bold.ttf')
    font = ImageFont.truetype(font_file, size=font_size)
    temp = []
    for i in range(len(word)):
        draw.text((10+i*30, -2), word[i], random_color(), font=font)
        temp.append(word[i])

    valid_str = "".join(temp)    # 验证码
    return valid_str, image

#增加噪点
def noise(image, width=120, height=35, line_count=3, point_count=20):
    draw = ImageDraw.Draw(image)
    for i in range(line_count):
        x1 = random.randint(0, width)
        x2 = random.randint(0, width)
        y1 = random.randint(0, height)
        y2 = random.randint(0, height)
        draw.line((x1, y1, x2, y2), fill=random_color())

        # 画点
        for i in range(point_count):
            draw.point([random.randint(0, width), random.randint(0, height)], fill=random_color())
            x = random.randint(0, width)
            y = random.randint(0, height)
            draw.arc((x, y, x + 4, y + 4), 0, 90, fill=random_color())

    return image

#生成图片验证码，且进行base64编码
def valid_code(word):
    image = generate_picture()
    valid_str, image = draw_str(word, image, 35)
    image = noise(image)

    f = BytesIO()
    image.save(f, 'png')        # 保存到BytesIO对象中, 格式为png
    data = f.getvalue()
    f.close()

    encode_data = base64.b64encode(data)
    data = str(encode_data, encoding='utf-8')
    img_data = "data:image/jpeg;base64,{data}".format(data=data)
    return img_data

if __name__ == '__main__':
    if(len(sys.argv) >= 2):
        filename = sys.argv[1];
        with open(filename,"w+") as file:
            file.write(valid_code(filename))
        