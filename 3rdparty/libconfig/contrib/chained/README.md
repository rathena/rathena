# Method Chained libconfig #

**Exception free + header only + method chained + config files**

Provides reading the configuration and defining the configuration specification at once.

### Features ###

 * default values
 * limits (min/max)
 * mandatory/optional values
 * help text output for expected config format on specification violation
 * capturing and outputting expected configuration specification/defaults

While it is possible to write a config file with this extension using the configuration specification capturing feature, it is not intended to be used as a full fledged config writer.

### Example ###

```C++
#include <libconfig_chained.h>

using namespace std;
using namespace libconfig;

int main(int argc, char **argv)
{
    Config cfg;
    cfg.readFile("example.cfg");
    ChainedSetting cs(cfg.getRoot());

    string name = cs["name"].defaultValue("<name>").isMandatory();
    string abstract = cs["abstract"].defaultValue("<unknown>");
    double longitude = cs["longitude"].min(-180.0).max(180.0).isMandatory();
    double latitude = cs["latitude"].min(-90.0).max(90.0).isMandatory();

    if (cs.isAnyMandatorySettingMissing())
    {
        cerr << "Cannot proceed until all mandatory settings are set." << endl;
    }
}
```

Console Output:
```sh
'longitude' setting is out of valid bounds (max: 180). Value was: 1200.35
Missing 'latitude' setting in configuration file.
Cannot proceed until all mandatory settings are set.
```

---

### How to integrate into your project ###

 1. Link the libconfig++.[lib/la/a] library as usual (see standard use of libconfig++).
 * Replace any includes of libconfig.h++ by libconfig_chained.h.
 * Use method chained candy as displayed above.

---

Create an issue for any questions or suggestions. Alternatively email me at github [at) hemofektik.de