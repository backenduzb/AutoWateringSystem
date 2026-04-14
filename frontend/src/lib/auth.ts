import { user, isAuthenticated, loading } from "../stores/auth";
import { ROOT_URL } from "$lib";
import { goto } from "$app/navigation";

export async function checkAuth() {
  loading.set(true);

  const token = localStorage.getItem("access");
  if (!token) {
    isAuthenticated.set(false);
    user.set(null);
    loading.set(false);
    return;
  }

  try {
    const res = await fetch(`${ROOT_URL}/accounts/me/`, {
      headers: {
        Authorization: `Token ${token}`,
        "Content-Type": "application/json",
      },
    });

    if (!res.ok) {
      throw new Error("Unauthorized");
    }

    const data = await res.json();
    user.set(data);
    isAuthenticated.set(true);
  } catch (err) {
    console.log("Auth failed:", err);
    user.set(null);
    isAuthenticated.set(false);
  } finally {
    loading.set(false);
  }
}

export function logout() {
  localStorage.removeItem("access");
  localStorage.removeItem("refresh");
  user.set(null);
  isAuthenticated.set(false);
  goto("/authenticate");
}