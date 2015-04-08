# run to extract tars needed:
# extracts:
#   GMM input data
#   ASR pretrained model
#   Stemmer larger input

find . -name '*.tar.gz' -exec sh -c 'tar -xzvf {} -C $(dirname {})' \;
