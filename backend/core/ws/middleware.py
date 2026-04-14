from __future__ import annotations

from typing import Callable, Awaitable, Optional
from urllib.parse import parse_qs

from asgiref.sync import sync_to_async
from channels.middleware import BaseMiddleware
from django.contrib.auth import get_user_model
from django.contrib.auth.models import AnonymousUser
from rest_framework_simplejwt.exceptions import InvalidToken, TokenError
from rest_framework_simplejwt.settings import api_settings
from rest_framework_simplejwt.tokens import UntypedToken


@sync_to_async
def _get_user(user_id: int):
    User = get_user_model()
    try:
        return User.objects.get(id=user_id)
    except User.DoesNotExist:
        return AnonymousUser()


def _get_token_from_scope(scope) -> Optional[str]:
    query = parse_qs(scope.get("query_string", b"").decode("utf-8"))
    token = query.get("token", [None])[0]
    if token:
        return token

    for key, value in scope.get("headers", []):
        if key.lower() == b"authorization":
            raw = value.decode("utf-8")
            parts = raw.split(" ", 1)
            if len(parts) == 2:
                return parts[1].strip()
            return raw.strip()
    return None


class JwtAuthMiddleware(BaseMiddleware):
    async def __call__(self, scope, receive, send):
        token = _get_token_from_scope(scope)
        scope["user"] = AnonymousUser()

        if token:
            try:
                UntypedToken(token)
                decoded = api_settings.TOKEN_BACKEND.decode(token, verify=True)
                user_id = decoded.get(api_settings.USER_ID_CLAIM)
                if user_id is not None:
                    scope["user"] = await _get_user(int(user_id))
            except (InvalidToken, TokenError, ValueError):
                scope["user"] = AnonymousUser()

        return await super().__call__(scope, receive, send)

