import { writable } from "svelte/store";

export interface Department {
  id: number;
  name: string;
}

export interface UserData {
  id: number;
  first_name: string;
  last_name: string;
  username: string;
  email?: string;
  department?: Department | null;
}

export interface UserResponse {
  data: UserData;
}

export const user = writable<UserResponse | null>(null);
export const isAuthenticated = writable(false);
export const loading = writable(true);