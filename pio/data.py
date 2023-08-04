import gzip
import os

os.makedirs('.pio/data', exist_ok=True)

for filename in ['config.html', 'logo-icon.png', 'logo.png']:
    with open('data/' + filename, 'rb') as f_in:
        with gzip.open('.pio/data/' + filename + '.gz', 'wb') as f_out:
            print('gzip \'data/' + filename + '\' to \'.pio/data/' + filename + '.gz\'')
            f_out.writelines(f_in)
