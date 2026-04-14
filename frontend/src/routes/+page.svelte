<script>
  import { Label } from 'flowbite-svelte';
  import Bakcground from '$lib/assets/background.avif';
  import Logo from '$lib/assets/logo.png';
  import LoginComponent from '$lib/components/authorization/Login.svelte';
</script>

<div
  class="min-h-screen flex bg-black bg-cover bg-center"
  style={`background-image: url('${Bakcground}');`}
>
  <div class="hidden sm:flex sm:basis-1/2 sm:flex-none items-center justify-center backdrop-blur-md bg-black/10">
    <div>
      <Label class="text-4xl font-bold">
        Avtomatik sug'orish tizimi
      </Label>

      <div class="flex items-center mt-10 gap-4">
        <img src={Logo} alt="logo" width="45" height="45" />
        <Label>
          Qishloq ho'jaligi fermerlik, dalalari va issiqhonalar uchun
          mukammallashtirilgan avtomatik boshqaruv tizimi.
        </Label>
      </div>
    </div>
  </div>
  <div class="basis-full sm:basis-1/2 flex items-center justify-center">
    <LoginComponent />
  </div>
</div>