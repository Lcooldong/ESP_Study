Import("env")

# Automatically run 'uploadfs' before normal upload
def before_upload_fs(source, target, env):
    print("Uploading LittleFS image...")
    env.Execute("pio run --target uploadfs")

env.AddPreAction("upload", before_upload_fs)