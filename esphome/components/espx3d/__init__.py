import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

# Dépendances
DEPENDENCIES = []

# Déclaration des classes C++
CC1101_X2D = cg.global_ns.class_("CC1101_X3D", cg.Component)
X3DWellcomRemote = cg.global_ns.class_("X3DWellcomRemote", cg.Component)

CONF_CSN_PIN = "csn_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_WELLCOM_REMOTES = "wellcom_remotes"

CONF_DEVICE_ID = "device_id"
CONF_NETWORK_ID = "network_id"

WELLCOM_REMOTE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(X3DWellcomRemote),
        cv.Required(CONF_DEVICE_ID): cv.uint32_t,
        cv.Required(CONF_NETWORK_ID): cv.uint8_t,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CC1101_X2D),
        cv.Required(CONF_CSN_PIN): pins.internal_gpio_pin_number,
        cv.Required(CONF_GDO0_PIN): pins.internal_gpio_pin_number,
        cv.Required(CONF_GDO2_PIN): pins.internal_gpio_pin_number,
        cv.Optional(CONF_WELLCOM_REMOTES): cv.ensure_list(WELLCOM_REMOTE_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    cg.add_library("RadioLib", "5.6.0")

    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_CSN_PIN],
        config[CONF_GDO0_PIN],
        config[CONF_GDO2_PIN],
    )
    yield cg.register_component(var, config)

    if CONF_WELLCOM_REMOTES in config:
        for remote_conf in config[CONF_WELLCOM_REMOTES]:
            remote_var = cg.new_Pvariable(
                remote_conf[CONF_ID],
                var,
                remote_conf[CONF_DEVICE_ID],
                remote_conf[CONF_NETWORK_ID]
            )
            yield cg.register_component(remote_var, remote_conf)
