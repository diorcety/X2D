import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

# Dépendances
DEPENDENCIES = []

# Déclaration des classes C++
CC1101_X2D = cg.global_ns.class_("CC1101_X2D", cg.Component)
X2DHeatingLevelActuator = cg.global_ns.class_("X2DHeatingLevelActuator", cg.Component)

CONF_CSN_PIN = "csn_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_ACTUATORS = "actuators"
CONF_HOUSE_ID = "house_id"
CONF_ZONE_ID = "zone_id"

ACTUATOR_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(X2DHeatingLevelActuator),
        cv.Required(CONF_HOUSE_ID): cv.uint16_t,
        cv.Required(CONF_ZONE_ID): cv.uint8_t,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CC1101_X2D),
        cv.Required(CONF_CSN_PIN): pins.internal_gpio_pin_number,
        cv.Required(CONF_GDO0_PIN): pins.internal_gpio_pin_number,
        cv.Required(CONF_GDO2_PIN): pins.internal_gpio_pin_number,
        cv.Optional(CONF_ACTUATORS): cv.ensure_list(ACTUATOR_SCHEMA),
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

    if CONF_ACTUATORS in config:
        for act_conf in config[CONF_ACTUATORS]:
            act_var = cg.new_Pvariable(
                act_conf[CONF_ID], var, act_conf[CONF_HOUSE_ID], act_conf[CONF_ZONE_ID]
            )
            yield cg.register_component(act_var, act_conf)
