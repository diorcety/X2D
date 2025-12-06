import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import CC1101_X2D

X2DZoneLevelSensor = cg.global_ns.class_('X2DZoneLevelSensor', text_sensor.TextSensor, cg.Component)

CONF_X2D_ID = 'x2d_id'
CONF_HOUSE_ID = 'house_id'
CONF_ZONE_ID = 'zone_id'

CONFIG_SCHEMA = text_sensor.text_sensor_schema(X2DZoneLevelSensor).extend({
    cv.GenerateID(CONF_X2D_ID): cv.use_id(CC1101_X2D),
    cv.Required(CONF_HOUSE_ID): cv.uint16_t,
    cv.Required(CONF_ZONE_ID): cv.uint8_t,
})

async def to_code(config):
    cc1101_x2d = await cg.get_variable(config[CONF_X2D_ID])

    var = cg.new_Pvariable(config[CONF_ID], cc1101_x2d, config[CONF_HOUSE_ID], config[CONF_ZONE_ID])

    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)