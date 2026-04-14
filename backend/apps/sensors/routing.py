from django.urls import path

from .consumers import SensorConsumer, UpdatesConsumer

websocket_urlpatterns = [
    path("ws/updates/", UpdatesConsumer.as_asgi()),
    path("ws/sensor/", SensorConsumer.as_asgi()),
]

