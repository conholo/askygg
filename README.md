
# About
askygg (æskɪg, ASK-igg) is an image editor application, developed in C++ with an OpenGL graphics API backend, that offers two operational modes: headless and editor. 
![Yggdrasil Art - https://wallpapers.com/wallpapers/norse-mythology-x3yqc88yii052ppl.html](ygg.jpg)


# Usage
From the project root directory, open the settings.json file and update the fields.  A description of each field is outlined below.

<ul>
<li> `input_dir: The absolute path of the input directory.  In the case of editor mode, this directory should contain a smaller sample of the image batch you intend to process in headless mode.  In headless mode, this directory should contain the entire set of images you want processed.
<li> `ouput_dir`: The absolute path of the destination of the application output (the enhanced images).
<li> `config_file`: The absolute path to your configuration file.  By default, a configuration file calibrated for night-time enhancement will be provided in the root directory.
</ul>
Once you're satisfied with the settings.json file, run the following command to build the project:
<pre>
python setup.py
</pre>
Once setup is complete, run the following command to run in editor mode or headless mode respectively:
<pre>
python run.py --mode editor
python run.py --mode headless
</pre>


By default, run.py runs askygg in release mode, to run in debug
<pre>
python run.py --mode editor --build_type debug
python run.py --mode headless --build_type debug
</pre>

# Examples

| Before                                                      | After                                                                 |
|-------------------------------------------------------------|-----------------------------------------------------------------------|
| ![Monkey Before](docs/images/examples/monkey.png)           | ![Monkey After](docs/images/examples/monkey_processed.jpeg)           |
| ![Airport Night Before](docs/images/examples/sfo_night.jpg) | ![Airport Night After](docs/images/examples/sfo_night_processed.jpeg) |
| ![Lights Before](docs/images/examples/lights.jpg)           | ![Lights After](docs/images/examples/lights_processed.jpeg)           |
| ![Video Game Before](docs/images/examples/supermario64.png) | ![Video Game After](docs/images/examples/supermario64_processed.jpg)  |


## Editor Features

Our editor mode comes with several powerful features to give users fine-grained control over the image processing pipeline.

### Pass Execution Order
Users can reorder and select which passes should be applied to the image, allowing for customized processing workflows.

![Pass Execution Order](docs/images/execution_order.png)

The Execution Definition panel exposes the set of all available passes; where passes on the left are to be applied in the order (from top to bottom) that they appear.  Passes on the right are disabled - passes can be dragged and dropped to change their order or enable/disable them.

### Performance Monitor
The performance monitor provides real-time feedback on the computational cost of each active pass, helping users optimize their processing pipeline.

![Performance Monitor](docs/images/perf.png)

### Pass Editor
Each pass comes with its own set of adjustable parameters. The pass editor allows users to fine-tune these settings for optimal results.

![Pass Editor](docs/images/pass_inspector.png)

These settings will be saved to the current configuration file when the "Save" button is pressed.  This WILL overwrite existing data. 